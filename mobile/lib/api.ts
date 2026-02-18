// Base URL of the Bedrock API running in the Multipass VM.
// Change this if your VM IP changes (run: multipass info bedrock-starter).
const API_BASE = "http://192.168.2.7";

type DemoContext = {
  userID: number;
  chatID: number;
};

let demoContext: DemoContext | null = null;

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
  const res = await fetch(`${API_BASE}${path}`, {
    headers: { "Content-Type": "application/json" },
    ...options,
  });

  const data = await res.json();

  if (!res.ok) {
    throw new Error(data.error || `Request failed with status ${res.status}`);
  }

  return data;
}

async function ensureDemoContext(): Promise<DemoContext> {
  if (demoContext) {
    return demoContext;
  }

  const nonce = Date.now();
  const user = await request("/api/users", {
    method: "POST",
    body: JSON.stringify({
      email: `mobile-demo-${nonce}@example.com`,
      firstName: "Mobile",
      lastName: "Demo",
      displayName: "Mobile Demo",
    }),
  });

  const userID = Number(user.userID);
  const chat = await request("/api/chats", {
    method: "POST",
    body: JSON.stringify({
      creatorUserID: userID,
      title: "Polls Demo Chat",
    }),
  });

  demoContext = {
    userID,
    chatID: Number(chat.chatID),
  };
  return demoContext;
}

/** GET /api/chats/:chatID/polls — list polls in demo chat */
export async function getPolls(): Promise<PollSummary[]> {
  const context = await ensureDemoContext();
  const data = await request(
    `/api/chats/${context.chatID}/polls?requesterUserID=${context.userID}`
  );
  const polls = Array.isArray(data.polls) ? data.polls : [];
  return polls.map((poll: any) => ({
    pollID: Number(poll.pollID),
    question: String(poll.question ?? ""),
    createdAt: Number(poll.createdAt ?? 0),
    optionCount: Number(poll.optionCount ?? 0),
    totalVotes: Number(poll.totalVotes ?? 0),
  }));
}

/** GET /api/polls/:id?requesterUserID=:userID — get poll details */
export async function getPoll(pollID: number): Promise<PollDetail> {
  const context = await ensureDemoContext();
  const data = await request(
    `/api/polls/${pollID}?requesterUserID=${context.userID}`
  );

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

/** POST /api/chats/:chatID/polls — create a poll in demo chat */
export async function createPoll(question: string, options: string[]): Promise<{ pollID: string }> {
  const context = await ensureDemoContext();
  return request(`/api/chats/${context.chatID}/polls`, {
    method: "POST",
    body: JSON.stringify({
      creatorUserID: context.userID,
      question,
      type: "single_choice",
      allowChangeVote: false,
      isAnonymous: false,
      options,
    }),
  });
}

/** POST /api/polls/:id/votes — submit one selected option */
export async function submitVote(pollID: number, optionID: number): Promise<{ voteID: string }> {
  const context = await ensureDemoContext();
  return request(`/api/polls/${pollID}/votes`, {
    method: "POST",
    body: JSON.stringify({
      userID: context.userID,
      optionIDs: [optionID],
    }),
  });
}

export type { PollSummary, PollOption, PollDetail };
