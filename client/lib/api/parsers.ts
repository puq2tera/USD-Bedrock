import TypescriptUtils from "../TypescriptUtils";
import { UNKNOWN_USER_LABEL } from "./constants";
import type {
  ChatMember,
  ChatMessage,
  ChatSummary,
  PollOption,
  PollSummary,
  PollTextResponse,
  PollType,
  UserIdentity,
} from "./types";

export function parseJsonArray<T>(value: unknown, parser: (x: unknown) => T | null): T[] {
  return TypescriptUtils.parseArray(value, parser) ?? [];
}

export function parseBooleanLike(value: unknown): boolean {
  return TypescriptUtils.parseBoolean(value) ?? false;
}

export function parseUserIdentity(value: unknown): UserIdentity | null {
  if (!TypescriptUtils.isObject(value)) {
    return null;
  }

  const userID = TypescriptUtils.parseString(value.userID);
  if (TypescriptUtils.isNullOrWhiteSpace(userID)) {
    return null;
  }

  return {
    userID: userID as string,
    firstName: TypescriptUtils.parseString(value.firstName) ?? "",
    lastName: TypescriptUtils.parseString(value.lastName) ?? "",
    displayName: TypescriptUtils.parseString(value.displayName) ?? "",
  };
}

export function normalizeUserIdentity(identity: UserIdentity): UserIdentity {
  const displayName = TypescriptUtils.parseString(identity.displayName)?.trim() ?? "";
  const firstName = TypescriptUtils.parseString(identity.firstName)?.trim() ?? "";
  const lastName = TypescriptUtils.parseString(identity.lastName)?.trim() ?? "";

  return {
    userID: identity.userID,
    firstName,
    lastName,
    displayName: displayName || `${firstName} ${lastName}`.trim() || UNKNOWN_USER_LABEL,
  };
}

export function parseChatSummary(value: unknown): ChatSummary | null {
  if (!TypescriptUtils.isObject(value)) {
    return null;
  }

  const chatID = TypescriptUtils.parseString(value.chatID);
  if (TypescriptUtils.isNullOrWhiteSpace(chatID)) {
    return null;
  }

  return {
    chatID: chatID as string,
    title: TypescriptUtils.parseString(value.title) ?? "",
    createdAt: TypescriptUtils.parseString(value.createdAt) ?? "",
    createdByUserID: TypescriptUtils.parseString(value.createdByUserID) ?? "",
    requesterRole: TypescriptUtils.parseString(value.requesterRole) ?? "member",
  };
}

export function parseChatMessage(value: unknown): ChatMessage | null {
  if (!TypescriptUtils.isObject(value)) {
    return null;
  }

  const messageID = TypescriptUtils.parseString(value.messageID);
  const chatID = TypescriptUtils.parseString(value.chatID);
  const userID = TypescriptUtils.parseString(value.userID);
  if (TypescriptUtils.isNullOrWhiteSpace(messageID) || TypescriptUtils.isNullOrWhiteSpace(chatID) || TypescriptUtils.isNullOrWhiteSpace(userID)) {
    return null;
  }

  return {
    messageID: messageID as string,
    chatID: chatID as string,
    userID: userID as string,
    body: TypescriptUtils.parseString(value.body) ?? "",
    createdAt: TypescriptUtils.parseString(value.createdAt) ?? "",
    updatedAt: TypescriptUtils.parseString(value.updatedAt) ?? "",
  };
}

export function parsePollSummary(value: unknown): PollSummary | null {
  if (!TypescriptUtils.isObject(value)) {
    return null;
  }

  const pollID = TypescriptUtils.parseString(value.pollID);
  const chatID = TypescriptUtils.parseString(value.chatID);
  const type = TypescriptUtils.parseString(value.type) as PollType | null;
  const status = TypescriptUtils.parseString(value.status) as "open" | "closed" | null;
  if (!pollID || !chatID || !type || !status) {
    return null;
  }

  return {
    pollID,
    chatID,
    creatorUserID: TypescriptUtils.parseString(value.creatorUserID) ?? "",
    question: TypescriptUtils.parseString(value.question) ?? "",
    type,
    allowChangeVote: parseBooleanLike(value.allowChangeVote),
    isAnonymous: parseBooleanLike(value.isAnonymous),
    status,
    expiresAt: TypescriptUtils.parseString(value.expiresAt) ?? "",
    createdAt: TypescriptUtils.parseString(value.createdAt) ?? "",
    updatedAt: TypescriptUtils.parseString(value.updatedAt) ?? "",
    closedAt: TypescriptUtils.parseString(value.closedAt) ?? "",
    optionCount: TypescriptUtils.parseInteger(value.optionCount) ?? 0,
    totalVotes: TypescriptUtils.parseInteger(value.totalVotes) ?? 0,
    totalVoters: TypescriptUtils.parseInteger(value.totalVoters) ?? 0,
    responseCount: TypescriptUtils.parseInteger(value.responseCount) ?? 0,
    summaryMessageID: TypescriptUtils.parseString(value.summaryMessageID) ?? "",
  };
}

export function parsePollOption(value: unknown): PollOption | null {
  if (!TypescriptUtils.isObject(value)) {
    return null;
  }

  const optionID = TypescriptUtils.parseString(value.optionID);
  if (TypescriptUtils.isNullOrWhiteSpace(optionID)) {
    return null;
  }

  return {
    optionID: optionID as string,
    label: TypescriptUtils.parseString(value.label) ?? "",
    ord: TypescriptUtils.parseInteger(value.ord) ?? 0,
    isActive: parseBooleanLike(value.isActive),
    voteCount: TypescriptUtils.parseInteger(value.voteCount) ?? 0,
  };
}

export function parseTextResponse(value: unknown): PollTextResponse | null {
  if (!TypescriptUtils.isObject(value)) {
    return null;
  }

  const responseID = TypescriptUtils.parseString(value.responseID);
  const userID = TypescriptUtils.parseString(value.userID);
  if (TypescriptUtils.isNullOrWhiteSpace(responseID) || TypescriptUtils.isNullOrWhiteSpace(userID)) {
    return null;
  }

  return {
    responseID: responseID as string,
    userID: userID as string,
    textValue: TypescriptUtils.parseString(value.textValue) ?? "",
    createdAt: TypescriptUtils.parseString(value.createdAt) ?? "",
  };
}

export function parseRole(value: unknown): "owner" | "member" {
  return TypescriptUtils.parseString(value) === "owner" ? "owner" : "member";
}

export function parseChatMember(value: unknown): ChatMember | null {
  if (!TypescriptUtils.isObject(value)) {
    return null;
  }

  const memberUserID = TypescriptUtils.parseString(value.userID);
  const memberChatID = TypescriptUtils.parseString(value.chatID);
  if (TypescriptUtils.isNullOrWhiteSpace(memberUserID) || TypescriptUtils.isNullOrWhiteSpace(memberChatID)) {
    return null;
  }

  return {
    chatID: memberChatID as string,
    userID: memberUserID as string,
    role: parseRole(value.role),
    joinedAt: TypescriptUtils.parseString(value.joinedAt) ?? "",
  };
}
