import TypescriptUtils from "../TypescriptUtils";
import { UNKNOWN_USER_LABEL } from "./constants";
import { request } from "./http";
import { normalizeUserIdentity, parseJsonArray, parseUserIdentity } from "./parsers";
import type { UserIdentity } from "./types";

const identityCache = new Map<string, UserIdentity | null>();

export function cacheIdentity(identity: UserIdentity) {
  identityCache.set(identity.userID, normalizeUserIdentity(identity));
}

export async function getUserById(userID: string): Promise<UserIdentity | null> {
  if (TypescriptUtils.isNullOrWhiteSpace(userID)) {
    return null;
  }

  const cachedIdentity = identityCache.get(userID);
  if (cachedIdentity !== undefined) {
    return cachedIdentity;
  }

  try {
    const data = await request<Record<string, unknown>>(`/api/users/${userID}`);
    const identity = parseUserIdentity(data);
    if (!identity) {
      identityCache.set(userID, null);
      return null;
    }

    cacheIdentity(identity);
    return identityCache.get(userID) ?? null;
  } catch (error: any) {
    const message = TypescriptUtils.parseString(error?.message) ?? "";
    if (message.toLowerCase().includes("not found")) {
      identityCache.set(userID, null);
      return null;
    }
    throw error;
  }
}

export async function lookupUsers(userIDs: string[]): Promise<UserIdentity[]> {
  const normalizedUserIDs = TypescriptUtils.dedupe(
    userIDs
      .map((userID) => TypescriptUtils.parseString(userID)?.trim() ?? "")
      .filter((userID) => !TypescriptUtils.isNullOrWhiteSpace(userID))
  );

  if (normalizedUserIDs.length < 1) {
    return [];
  }

  const unresolvedUserIDs = normalizedUserIDs.filter((userID) => !identityCache.has(userID));
  if (unresolvedUserIDs.length > 0) {
    const data = await request<{ users?: unknown }>("/api/users/lookup", {
      method: "POST",
      body: JSON.stringify({ userIDs: unresolvedUserIDs }),
    });

    const users = parseJsonArray(data.users, parseUserIdentity).map(normalizeUserIdentity);
    const resolvedByID = new Set<string>();
    for (const user of users) {
      resolvedByID.add(user.userID);
      identityCache.set(user.userID, user);
    }

    for (const userID of unresolvedUserIDs) {
      if (!resolvedByID.has(userID)) {
        // Cache miss sentinels to avoid repeated lookup requests for known-unresolvable IDs.
        identityCache.set(userID, null);
      }
    }
  }

  return normalizedUserIDs
    .map((userID) => identityCache.get(userID) ?? null)
    .filter((identity): identity is UserIdentity => identity != null);
}

export async function lookupUserByEmail(email: string): Promise<UserIdentity | null> {
  const normalizedEmail = TypescriptUtils.parseString(email)?.trim().toLowerCase() ?? "";
  if (TypescriptUtils.isNullOrWhiteSpace(normalizedEmail)) {
    return null;
  }

  const data = await request<Record<string, unknown>>(`/api/users/by-email?email=${encodeURIComponent(normalizedEmail)}`);
  const identity = parseUserIdentity(data);
  if (!identity) {
    return null;
  }

  cacheIdentity(identity);
  return identity;
}

export async function hydrateUserIdentities(userIDs: string[]): Promise<void> {
  await lookupUsers(userIDs);
}

export function getCachedUserIdentity(userID: string): UserIdentity | null {
  if (TypescriptUtils.isNullOrWhiteSpace(userID)) {
    return null;
  }

  return identityCache.get(userID) ?? null;
}

export function getIdentityLabel(userID: string): string {
  const identity = getCachedUserIdentity(userID);
  if (!identity) {
    return UNKNOWN_USER_LABEL;
  }

  const displayName = TypescriptUtils.parseString(identity.displayName)?.trim() ?? "";
  if (!TypescriptUtils.isNullOrWhiteSpace(displayName)) {
    return displayName;
  }

  const fullName = `${identity.firstName} ${identity.lastName}`.trim();
  return fullName || UNKNOWN_USER_LABEL;
}
