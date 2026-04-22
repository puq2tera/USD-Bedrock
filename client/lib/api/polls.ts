import TypescriptUtils from "../TypescriptUtils";
import { request } from "./http";
import { hydrateUserIdentities } from "./identity";
import { parseBooleanLike, parseJsonArray, parsePollOption, parsePollSummary, parseTextResponse } from "./parsers";
import type {
  CreatePollInput,
  EditPollInput,
  PollDetail,
  PollParticipation,
  PollSummary,
  PollType,
  PollVotePayload,
} from "./types";

export async function listChatPolls(chatID: string): Promise<PollSummary[]> {
  const data = await request<{ polls?: unknown }>(`/api/chats/${chatID}/polls?includeClosed=true`);
  const polls = parseJsonArray(data.polls, parsePollSummary);
  await hydrateUserIdentities(polls.map((poll) => poll.creatorUserID));
  return polls;
}

export async function getPoll(pollID: string): Promise<PollDetail> {
  const data = await request<Record<string, unknown>>(`/api/polls/${pollID}`);
  const base = parsePollSummary(data);
  if (!base) {
    throw new Error("Invalid poll response");
  }

  const options = parseJsonArray(data.options, parsePollOption);
  const responses = parseJsonArray(data.responses, parseTextResponse);
  await hydrateUserIdentities([base.creatorUserID, ...responses.map((response) => response.userID)]);

  return {
    ...base,
    options,
    responses,
  };
}

export async function getPollParticipation(pollID: string): Promise<PollParticipation> {
  const data = await request<Record<string, unknown>>(`/api/polls/${pollID}/participation`);

  const result: PollParticipation = {
    pollID: TypescriptUtils.parseString(data.pollID) ?? pollID,
    chatID: TypescriptUtils.parseString(data.chatID) ?? "",
    type: (TypescriptUtils.parseString(data.type) as PollType | null) ?? "single_choice",
    isAnonymous: parseBooleanLike(data.isAnonymous),
    eligibleCount: TypescriptUtils.parseInteger(data.eligibleCount) ?? 0,
    votedCount: TypescriptUtils.parseInteger(data.votedCount) ?? 0,
    notVotedCount: TypescriptUtils.parseInteger(data.notVotedCount) ?? 0,
    eligibleUserIDs: parseJsonArray(data.eligibleUserIDs, (item) => {
      const parsed = TypescriptUtils.parseString(item);
      return TypescriptUtils.isNullOrWhiteSpace(parsed) ? null : (parsed as string);
    }),
    votedUserIDs: parseJsonArray(data.votedUserIDs, (item) => {
      const parsed = TypescriptUtils.parseString(item);
      return TypescriptUtils.isNullOrWhiteSpace(parsed) ? null : (parsed as string);
    }),
    notVotedUserIDs: parseJsonArray(data.notVotedUserIDs, (item) => {
      const parsed = TypescriptUtils.parseString(item);
      return TypescriptUtils.isNullOrWhiteSpace(parsed) ? null : (parsed as string);
    }),
  };

  if (!result.isAnonymous) {
    await hydrateUserIdentities([...result.eligibleUserIDs, ...result.votedUserIDs, ...result.notVotedUserIDs]);
  }

  return result;
}

export async function createPoll(chatID: string, input: CreatePollInput): Promise<{ pollID: string }> {
  const body: Record<string, unknown> = {
    question: input.question,
    type: input.type,
    allowChangeVote: input.allowChangeVote,
    isAnonymous: input.isAnonymous,
  };

  if (typeof input.expiresAt === "number" && input.expiresAt > 0) {
    body.expiresAt = input.expiresAt;
  }
  if (Array.isArray(input.options)) {
    body.options = input.options;
  }

  const data = await request<Record<string, unknown>>(`/api/chats/${chatID}/polls`, {
    method: "POST",
    body: JSON.stringify(body),
  });

  return { pollID: TypescriptUtils.parseString(data.pollID) ?? "" };
}

export async function editPoll(pollID: string, input: EditPollInput): Promise<void> {
  const body: Record<string, unknown> = {};
  if (!TypescriptUtils.isNullOrWhiteSpace(input.question)) {
    body.question = TypescriptUtils.parseString(input.question)?.trim() ?? "";
  }
  if (typeof input.allowChangeVote === "boolean") {
    body.allowChangeVote = input.allowChangeVote;
  }
  if (typeof input.isAnonymous === "boolean") {
    body.isAnonymous = input.isAnonymous;
  }
  if (!TypescriptUtils.isNullOrWhiteSpace(input.status)) {
    body.status = input.status;
  }
  if (input.expiresAt === null) {
    body.expiresAt = null;
  } else if (typeof input.expiresAt === "number") {
    body.expiresAt = input.expiresAt;
  }
  if (Array.isArray(input.options)) {
    body.options = input.options;
  }

  await request(`/api/polls/${pollID}`, {
    method: "PUT",
    body: JSON.stringify(body),
  });
}

export async function deletePoll(pollID: string): Promise<void> {
  await request(`/api/polls/${pollID}`, { method: "DELETE" });
}

export async function submitPollVotes(pollID: string, payload: PollVotePayload): Promise<void> {
  await request(`/api/polls/${pollID}/votes`, {
    method: "POST",
    body: JSON.stringify({ optionIDs: payload.optionIDs }),
  });
}

export async function deletePollVotes(pollID: string): Promise<void> {
  await request(`/api/polls/${pollID}/votes`, { method: "DELETE" });
}

export async function submitPollTextResponse(pollID: string, textValue: string): Promise<void> {
  await request(`/api/polls/${pollID}/responses`, {
    method: "POST",
    body: JSON.stringify({ textValue }),
  });
}
