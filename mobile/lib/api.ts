// Base URL of the Bedrock API running in the Multipass VM.
// Change this if your VM IP changes (run: multipass info bedrock-starter).
const API_BASE = "http://192.168.2.7";

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

/** GET /api/polls — list all polls */
export async function getPolls(): Promise<PollSummary[]> {
  const data = await request("/api/polls");
  return data.polls ?? [];
}

/** GET /api/polls/:id — get a single poll with options and vote counts */
export async function getPoll(pollID: number): Promise<PollDetail> {
  return request(`/api/polls/${pollID}`);
}

/** POST /api/polls — create a new poll */
export async function createPoll(question: string, options: string[]): Promise<{ pollID: string }> {
  return request("/api/polls", {
    method: "POST",
    body: JSON.stringify({ question, options: JSON.stringify(options) }),
  });
}

/** POST /api/polls/:id/vote — vote on a poll option */
export async function submitVote(pollID: number, optionID: number): Promise<{ voteID: string }> {
  return request(`/api/polls/${pollID}/vote`, {
    method: "POST",
    body: JSON.stringify({ optionID: String(optionID) }),
  });
}

export type { PollSummary, PollOption, PollDetail };
