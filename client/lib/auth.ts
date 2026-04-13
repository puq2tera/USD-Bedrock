import { Buffer } from "buffer";
import * as SecureStore from "expo-secure-store";
import { createContext, createElement, ReactNode, useContext, useEffect, useMemo, useState } from "react";
import TypescriptUtils from "./TypescriptUtils";

const API_BASE = process.env.EXPO_PUBLIC_API_BASE || "http://192.168.2.7";
const AUTH_STORAGE_KEY = "bedrock_auth_state";
const EXPIRY_SKEW_SECONDS = 60;

type AuthStatus = "loading" | "authenticated" | "unauthenticated";

type AuthUser = {
  userID: string;
  email: string;
  firstName: string;
  lastName: string;
  displayName: string;
};

type StoredAuthState = {
  accessToken: string;
  refreshToken: string;
  accessTokenExpiresAt: number;
  user: AuthUser;
};

type AuthSnapshot = {
  status: AuthStatus;
  accessToken: string | null;
  refreshToken: string | null;
  accessTokenExpiresAt: number | null;
  user: AuthUser | null;
};

type AuthContextValue = AuthSnapshot & {
  isAuthenticated: boolean;
  login: (email: string, password: string) => Promise<void>;
  register: (input: RegisterInput) => Promise<void>;
  logout: () => Promise<void>;
};

type LoginResponse = {
  accessToken: string;
  refreshToken: string;
  user: AuthUser;
};

type RefreshResponse = {
  accessToken: string;
  refreshToken: string;
};

type RegisterInput = {
  email: string;
  password: string;
  firstName: string;
  lastName: string;
  displayName?: string;
};

const AuthContext = createContext<AuthContextValue | null>(null);

let authSnapshot: AuthSnapshot = {
  status: "loading",
  accessToken: null,
  refreshToken: null,
  accessTokenExpiresAt: null,
  user: null,
};

let bootstrapPromise: Promise<void> | null = null;
let refreshPromise: Promise<boolean> | null = null;
const subscribers = new Set<() => void>();

function notifySubscribers() {
  for (const listener of subscribers) {
    listener();
  }
}

function setSnapshot(nextSnapshot: AuthSnapshot) {
  authSnapshot = nextSnapshot;
  notifySubscribers();
}

function snapshotFromStoredState(storedState: StoredAuthState): AuthSnapshot {
  return {
    status: "authenticated",
    accessToken: storedState.accessToken,
    refreshToken: storedState.refreshToken,
    accessTokenExpiresAt: storedState.accessTokenExpiresAt,
    user: storedState.user,
  };
}

async function persistSnapshot(snapshot: AuthSnapshot) {
  if (
    snapshot.status === "authenticated" &&
    snapshot.accessToken &&
    snapshot.refreshToken &&
    snapshot.accessTokenExpiresAt &&
    snapshot.user
  ) {
    await SecureStore.setItemAsync(
      AUTH_STORAGE_KEY,
      JSON.stringify({
        accessToken: snapshot.accessToken,
        refreshToken: snapshot.refreshToken,
        accessTokenExpiresAt: snapshot.accessTokenExpiresAt,
        user: snapshot.user,
      } satisfies StoredAuthState)
    );
    return;
  }

  await SecureStore.deleteItemAsync(AUTH_STORAGE_KEY);
}

async function updateSnapshot(snapshot: AuthSnapshot) {
  setSnapshot(snapshot);
  await persistSnapshot(snapshot);
}

function decodeJwtExpiry(token: string): number {
  const parts = token.split(".");
  if (parts.length !== 3) {
    throw new Error("Invalid access token");
  }

  const normalized = parts[1].replace(/-/g, "+").replace(/_/g, "/");
  const padded = normalized.padEnd(normalized.length + ((4 - (normalized.length % 4)) % 4), "=");
  const decoder = typeof globalThis.atob === "function"
    ? globalThis.atob
    : (value: string) => Buffer.from(value, "base64").toString("binary");
  const payload = JSON.parse(decoder(padded)) as { exp?: number };
  const expiry = TypescriptUtils.parseInteger(payload.exp);

  if (expiry == null || expiry <= 0) {
    throw new Error("Invalid access token");
  }

  return expiry;
}

function authHeaders(accessToken?: string | null): HeadersInit {
  const headers: Record<string, string> = {};
  if (accessToken) {
    headers.Authorization = `Bearer ${accessToken}`;
  }
  return headers;
}

async function fetchJson<T>(path: string, options?: RequestInit): Promise<T> {
  const response = await fetch(`${API_BASE}${path}`, options);
  const data = await response.json();

  if (!response.ok) {
    throw new Error(data.error || `Request failed with status ${response.status}`);
  }

  return data as T;
}

async function applyLoginResponse(response: LoginResponse) {
  await updateSnapshot({
    status: "authenticated",
    accessToken: response.accessToken,
    refreshToken: response.refreshToken,
    accessTokenExpiresAt: decodeJwtExpiry(response.accessToken),
    user: response.user,
  });
}

async function clearSnapshot() {
  await updateSnapshot({
    status: "unauthenticated",
    accessToken: null,
    refreshToken: null,
    accessTokenExpiresAt: null,
    user: null,
  });
}

async function refreshTokensInternal(): Promise<boolean> {
  const refreshToken = authSnapshot.refreshToken;
  if (!refreshToken) {
    await clearSnapshot();
    return false;
  }

  try {
    const response = await fetchJson<RefreshResponse>("/api/auth/refresh", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ refreshToken }),
    });

    await updateSnapshot({
      status: "authenticated",
      accessToken: response.accessToken,
      refreshToken: response.refreshToken,
      accessTokenExpiresAt: decodeJwtExpiry(response.accessToken),
      // Refresh currently rotates tokens only, so preserve the last known user payload until a
      // new login or logout replaces it.
      user: authSnapshot.user,
    });
    return true;
  } catch {
    await clearSnapshot();
    return false;
  }
}

async function refreshTokens(): Promise<boolean> {
  if (refreshPromise) {
    // Share one in-flight refresh across callers so concurrent 401 repairs do not race token
    // rotation and accidentally clear otherwise valid auth state.
    return refreshPromise;
  }

  refreshPromise = refreshTokensInternal().finally(() => {
    refreshPromise = null;
  });
  return refreshPromise;
}

async function ensureFreshAccessToken(forceRefresh = false): Promise<string | null> {
  await initializeAuth();

  if (authSnapshot.status !== "authenticated" || !authSnapshot.accessToken) {
    return null;
  }

  const expiresAt = authSnapshot.accessTokenExpiresAt ?? 0;
  const needsRefresh = forceRefresh || expiresAt - EXPIRY_SKEW_SECONDS <= Math.floor(Date.now() / 1000);
  if (!needsRefresh) {
    return authSnapshot.accessToken;
  }

  return (await refreshTokens()) ? authSnapshot.accessToken : null;
}

export async function initializeAuth(): Promise<void> {
  if (bootstrapPromise) {
    return bootstrapPromise;
  }

  bootstrapPromise = (async () => {
    const storedValue = await SecureStore.getItemAsync(AUTH_STORAGE_KEY);
    if (!storedValue) {
      await clearSnapshot();
      return;
    }

    try {
      const parsed = JSON.parse(storedValue) as Partial<StoredAuthState>;
      if (
        TypescriptUtils.isNullOrWhiteSpace(parsed.accessToken) ||
        TypescriptUtils.isNullOrWhiteSpace(parsed.refreshToken) ||
        TypescriptUtils.parseInteger(parsed.accessTokenExpiresAt) == null ||
        !TypescriptUtils.isObject(parsed.user)
      ) {
        throw new Error("Invalid auth state");
      }

      const normalizedState: StoredAuthState = {
        accessToken: TypescriptUtils.parseString(parsed.accessToken) as string,
        refreshToken: TypescriptUtils.parseString(parsed.refreshToken) as string,
        accessTokenExpiresAt: TypescriptUtils.parseInteger(parsed.accessTokenExpiresAt) as number,
        user: TypescriptUtils.clone(parsed.user as AuthUser),
      };

      setSnapshot(snapshotFromStoredState(normalizedState));
      await persistSnapshot(authSnapshot);

      if (normalizedState.accessTokenExpiresAt - EXPIRY_SKEW_SECONDS <= Math.floor(Date.now() / 1000)) {
        await refreshTokens();
      }
    } catch {
      await clearSnapshot();
    }
  })().finally(() => {
    bootstrapPromise = null;
  });

  return bootstrapPromise;
}

export function getAuthSnapshot(): AuthSnapshot {
  return authSnapshot;
}

export function subscribeAuth(listener: () => void): () => void {
  subscribers.add(listener);
  return () => {
    subscribers.delete(listener);
  };
}

export async function login(email: string, password: string): Promise<void> {
  if (TypescriptUtils.isNullOrWhiteSpace(email) || TypescriptUtils.isNullOrEmpty(password)) {
    throw new Error("Email and password are required");
  }

  const response = await fetchJson<LoginResponse>("/api/auth/login", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({
      email: TypescriptUtils.parseString(email)?.trim(),
      password,
    }),
  });
  await applyLoginResponse(response);
}

export async function register(input: RegisterInput): Promise<void> {
  const normalizedEmail = TypescriptUtils.parseString(input.email)?.trim() ?? "";
  const normalizedFirstName = TypescriptUtils.parseString(input.firstName)?.trim() ?? "";
  const normalizedLastName = TypescriptUtils.parseString(input.lastName)?.trim() ?? "";
  const normalizedDisplayName = TypescriptUtils.parseString(input.displayName)?.trim() ?? undefined;

  if (
    TypescriptUtils.isNullOrWhiteSpace(normalizedEmail) ||
    TypescriptUtils.isNullOrWhiteSpace(normalizedFirstName) ||
    TypescriptUtils.isNullOrWhiteSpace(normalizedLastName) ||
    TypescriptUtils.isNullOrEmpty(input.password)
  ) {
    throw new Error("Registration fields are required");
  }

  await fetchJson("/api/users", {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({
      ...input,
      email: normalizedEmail,
      firstName: normalizedFirstName,
      lastName: normalizedLastName,
      displayName: normalizedDisplayName,
    }),
  });
  await login(normalizedEmail, input.password);
}

export async function logout(): Promise<void> {
  await initializeAuth();

  const accessToken = authSnapshot.accessToken;
  const refreshToken = authSnapshot.refreshToken;

  try {
    if (accessToken && refreshToken) {
      await fetch(`${API_BASE}/api/auth/logout`, {
        method: "POST",
        headers: {
          "Content-Type": "application/json",
          ...authHeaders(accessToken),
        },
        body: JSON.stringify({ refreshToken }),
      });
    }
  } finally {
    await clearSnapshot();
  }
}

export async function authFetch(input: string, init: RequestInit = {}): Promise<Response> {
  const firstToken = await ensureFreshAccessToken();
  const headers = new Headers(init.headers);
  if (firstToken) {
    headers.set("Authorization", `Bearer ${firstToken}`);
  }

  let response = await fetch(input, { ...init, headers });
  if (response.status !== 401) {
    return response;
  }

  const nextToken = await ensureFreshAccessToken(true);
  if (!nextToken) {
    return response;
  }

  const retryHeaders = new Headers(init.headers);
  retryHeaders.set("Authorization", `Bearer ${nextToken}`);
  response = await fetch(input, { ...init, headers: retryHeaders });
  return response;
}

export function AuthProvider({ children }: { children: ReactNode }) {
  const [snapshot, setSnapshotState] = useState<AuthSnapshot>(authSnapshot);

  useEffect(() => {
    initializeAuth().catch(() => {
      void clearSnapshot();
    });
    return subscribeAuth(() => setSnapshotState(getAuthSnapshot()));
  }, []);

  const value = useMemo<AuthContextValue>(
    () => ({
      ...snapshot,
      isAuthenticated: snapshot.status === "authenticated",
      login,
      register,
      logout,
    }),
    [snapshot]
  );

  return createElement(AuthContext.Provider, { value }, children);
}

export function useAuth(): AuthContextValue {
  const context = useContext(AuthContext);
  if (!context) {
    throw new Error("useAuth must be used within AuthProvider");
  }

  return context;
}

export type { AuthUser, RegisterInput };
