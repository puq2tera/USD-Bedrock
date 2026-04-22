import { getAuthSnapshot } from "../auth";
import type { ChatDetail, ChatMessage, PollDetail, PollSummary } from "./types";

function authUserID(): string {
  return getAuthSnapshot().user?.userID ?? "";
}

export function canManageChat(chat: ChatDetail): boolean {
  return chat.requesterRole === "owner";
}

export function canManageMessage(message: ChatMessage): boolean {
  return message.userID === authUserID();
}

export function canManagePoll(poll: PollSummary | PollDetail): boolean {
  return poll.creatorUserID === authUserID();
}

export function canEditPollVotes(poll: PollSummary | PollDetail): boolean {
  if (poll.status !== "open") {
    return false;
  }

  return poll.allowChangeVote || poll.totalVoters < 1;
}
