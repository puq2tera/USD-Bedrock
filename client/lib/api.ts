import { authFetch, getAuthSnapshot, type AuthUser } from "./auth";
import TypescriptUtils from "./TypescriptUtils";

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
    // The cached chat belongs to one authenticated user. Drop it as soon as auth switches users so
    // later requests cannot accidentally reuse another account's chat context.
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
  const chats = TypescriptUtils.parseArray(chatsData.chats, (item) =>
    TypescriptUtils.isObject(item) ? (item as { chatID?: string | number }) : null
  ) ?? [];
  if (chats.length > 0) {
    const chatID = TypescriptUtils.parseInteger(chats[0].chatID);
    if (chatID != null) {
      activeChatID = chatID;
      return activeChatID;
    }
  }

  const created = await request<{ chatID: string | number }>("/api/chats", {
    method: "POST",
    body: JSON.stringify({ title: "My Polls" }),
  });
  const createdChatID = TypescriptUtils.parseInteger(created.chatID);
  if (createdChatID == null) {
    throw new Error("API returned an invalid chat ID");
  }

  activeChatID = createdChatID;
  return activeChatID;
}

export async function getPolls(): Promise<PollSummary[]> {
  const chatID = await ensureActiveChatID();
  const data = await request<{ polls?: unknown }>(`/api/chats/${chatID}/polls`);
  const polls = TypescriptUtils.parseArray(data.polls, (poll) => {
    if (!TypescriptUtils.isObject(poll)) {
      return null;
    }

    return {
      pollID: TypescriptUtils.parseInteger(poll.pollID) ?? 0,
      question: TypescriptUtils.parseString(poll.question) ?? "",
      createdAt: TypescriptUtils.parseInteger(poll.createdAt) ?? 0,
      optionCount: TypescriptUtils.parseInteger(poll.optionCount) ?? 0,
      totalVotes: TypescriptUtils.parseInteger(poll.totalVotes) ?? 0,
    } satisfies PollSummary;
  }) ?? [];

  return polls.filter((poll) => poll.pollID > 0);
}

export async function getPoll(pollID: number): Promise<PollDetail> {
  const data = await request<Record<string, unknown>>(`/api/polls/${pollID}`);
  const options = TypescriptUtils.parseArray(data.options, (option) => {
    if (!TypescriptUtils.isObject(option)) {
      return null;
    }

    return {
      optionID: TypescriptUtils.parseInteger(option.optionID) ?? 0,
      text: TypescriptUtils.parseString(option.label ?? option.text) ?? "",
      votes: TypescriptUtils.parseInteger(option.voteCount ?? option.votes) ?? 0,
    } satisfies PollOption;
  }) ?? [];

  return {
    pollID: TypescriptUtils.parseString(data.pollID) ?? "",
    question: TypescriptUtils.parseString(data.question) ?? "",
    createdAt: TypescriptUtils.parseString(data.createdAt) ?? "",
    optionCount: TypescriptUtils.parseString(data.optionCount) ?? "0",
    totalVotes: TypescriptUtils.parseString(data.totalVotes) ?? "0",
    options: options.filter((option) => option.optionID > 0),
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
