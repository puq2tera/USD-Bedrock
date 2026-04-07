import { authFetch, getAuthSnapshot, type AuthUser } from "./auth";

// Base URL of the Bedrock API running in the Multipass VM.
// Override via EXPO_PUBLIC_API_BASE in client/.env (see .env.example).
const API_BASE = process.env.EXPO_PUBLIC_API_BASE || "http://192.168.2.7";

let activeChatID: number | null = null;
let activeChatUserID: string | null = null;

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

async function request<T>(path: string, options?: RequestInit): Promise<T> {
  const headers: Record<string, string> = {
    "Content-Type": "application/json",
    ...(options?.headers as Record<string, string> | undefined),
  };

  const response = await authFetch(`${API_BASE}${path}`, {
    ...options,
    headers,
  });
  const data = await response.json();

  if (!response.ok) {
    throw new Error(data.error || `Request failed with status ${response.status}`);
  }

  return data as T;
}

function resetActiveChatIfUserChanged(user: AuthUser | null) {
  const nextUserID = user?.userID ?? null;
  if (activeChatUserID !== nextUserID) {
    activeChatID = null;
    activeChatUserID = nextUserID;
  }
}

async function ensureActiveChatID(): Promise<number> {
  resetActiveChatIfUserChanged(getAuthSnapshot().user);
  if (activeChatID !== null) {
    return activeChatID;
  }

  const chatsData = await request<{ chats?: Array<{ chatID?: string | number }> }>("/api/chats");
  const chats = Array.isArray(chatsData.chats) ? chatsData.chats : [];
  if (chats.length > 0) {
    activeChatID = Number(chats[0].chatID);
    return activeChatID;
  }

  const created = await request<{ chatID: string | number }>("/api/chats", {
    method: "POST",
    body: JSON.stringify({ title: "My Polls" }),
  });
  activeChatID = Number(created.chatID);
  return activeChatID;
}

export async function getPolls(): Promise<PollSummary[]> {
  const chatID = await ensureActiveChatID();
  const data = await request<{ polls?: any[] }>(`/api/chats/${chatID}/polls`);
  const polls = Array.isArray(data.polls) ? data.polls : [];
  return polls.map((poll) => ({
    pollID: Number(poll.pollID),
    question: String(poll.question ?? ""),
    createdAt: Number(poll.createdAt ?? 0),
    optionCount: Number(poll.optionCount ?? 0),
    totalVotes: Number(poll.totalVotes ?? 0),
  }));
}

export async function getPoll(pollID: number): Promise<PollDetail> {
  const data = await request<any>(`/api/polls/${pollID}`);

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

export async function submitVote(pollID: number, optionID: number): Promise<{ voteID: string }> {
  return request(`/api/polls/${pollID}/votes`, {
    method: "POST",
    body: JSON.stringify({
      optionIDs: [optionID],
    }),
  });
}

export type { PollSummary, PollOption, PollDetail };
