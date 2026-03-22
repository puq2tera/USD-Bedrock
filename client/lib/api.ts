import * as SecureStore from "expo-secure-store";

// Base URL of the Bedrock API running in the Multipass VM.
// Override via EXPO_PUBLIC_API_BASE in client/.env (see .env.example).
const API_BASE = process.env.EXPO_PUBLIC_API_BASE || "http://192.168.2.7";
const AUTH_TOKEN_KEY = "bedrock_auth_token";

let authToken: string | null = null;
let activeChatID: number | null = null;

type PollSummary = {
  pollID: number;
  question: string;
  createdAt: number;
  optionCount: number;
  totalVotes: number;
};

type PollOption = {
  optionID: number;
  text: string;
  votes: number;
};

type PollDetail = {
  pollID: string;
  question: string;
  createdAt: string;
  options: PollOption[];
  optionCount: string;
  totalVotes: string;
};

async function request(path: string, options?: RequestInit) {
  const headers: Record<string, string> = {
    "Content-Type": "application/json",
  };
  if (authToken) {
    headers.Authorization = `Bearer ${authToken}`;
  }

  const res = await fetch(`${API_BASE}${path}`, {
    headers,
    ...options,
  });

  const data = await res.json();

  if (!res.ok) {
    throw new Error(data.error || `Request failed with status ${res.status}`);
  }

  return data;
}

export async function initializeAuth(): Promise<void> {
  const token = await SecureStore.getItemAsync(AUTH_TOKEN_KEY);
  authToken = token && token.length > 0 ? token : null;
  activeChatID = null;
}

export function hasToken(): boolean {
  return authToken !== null;
}

async function setToken(token: string): Promise<void> {
  authToken = token;
  activeChatID = null;
  await SecureStore.setItemAsync(AUTH_TOKEN_KEY, token);
}

export async function logout(): Promise<void> {
  authToken = null;
  activeChatID = null;
  await SecureStore.deleteItemAsync(AUTH_TOKEN_KEY);
}

export async function login(email: string, password: string): Promise<void> {
  const data = await request("/api/auth/login", {
    method: "POST",
    body: JSON.stringify({ email, password }),
  });
  await setToken(String(data.token));
}

export async function register(input: {
  email: string;
  password: string;
  firstName: string;
  lastName: string;
  displayName?: string;
}): Promise<void> {
  await request("/api/users", {
    method: "POST",
    body: JSON.stringify(input),
  });
  await login(input.email, input.password);
}

async function ensureActiveChatID(): Promise<number> {
  if (activeChatID !== null) {
    return activeChatID;
  }

  const chatsData = await request("/api/chats");
  const chats = Array.isArray(chatsData.chats) ? chatsData.chats : [];
  if (chats.length > 0) {
    activeChatID = Number(chats[0].chatID);
    return activeChatID;
  }

  const created = await request("/api/chats", {
    method: "POST",
    body: JSON.stringify({ title: "My Polls" }),
  });
  activeChatID = Number(created.chatID);
  return activeChatID;
}

/** GET /api/chats/:chatID/polls — list polls in active user chat */
export async function getPolls(): Promise<PollSummary[]> {
  const chatID = await ensureActiveChatID();
  const data = await request(`/api/chats/${chatID}/polls`);
  const polls = Array.isArray(data.polls) ? data.polls : [];
  return polls.map((poll: any) => ({
    pollID: Number(poll.pollID),
    question: String(poll.question ?? ""),
    createdAt: Number(poll.createdAt ?? 0),
    optionCount: Number(poll.optionCount ?? 0),
    totalVotes: Number(poll.totalVotes ?? 0),
  }));
}

/** GET /api/polls/:id — get poll details */
export async function getPoll(pollID: number): Promise<PollDetail> {
  const data = await request(`/api/polls/${pollID}`);

  const options = Array.isArray(data.options) ? data.options : [];
  return {
    ...data,
    options: options.map((option: any) => ({
      optionID: Number(option.optionID),
      text: String(option.label ?? option.text ?? ""),
      votes: Number(option.voteCount ?? option.votes ?? 0),
    })),
  };
}

/** POST /api/chats/:chatID/polls — create a poll in active user chat */
export async function createPoll(question: string, options: string[]): Promise<{ pollID: string }> {
  const chatID = await ensureActiveChatID();
  return request(`/api/chats/${chatID}/polls`, {
    method: "POST",
    body: JSON.stringify({
      question,
      type: "single_choice",
      allowChangeVote: true,
      isAnonymous: false,
      options,
    }),
  });
}

/** POST /api/polls/:id/votes — submit one selected option */
export async function submitVote(pollID: number, optionID: number): Promise<{ voteID: string }> {
  return request(`/api/polls/${pollID}/votes`, {
    method: "POST",
    body: JSON.stringify({
      optionIDs: [optionID],
    }),
  });
}

export type { PollSummary, PollOption, PollDetail };
