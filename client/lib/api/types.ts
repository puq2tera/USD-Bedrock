export type UserIdentity = {
  userID: string;
  firstName: string;
  lastName: string;
  displayName: string;
};

export type AccountProfile = UserIdentity & {
  email: string;
  createdAt: string;
};

export type UpdateAccountInput = {
  email?: string;
  firstName?: string;
  lastName?: string;
  displayName?: string;
};

export type ChatSummary = {
  chatID: string;
  title: string;
  createdAt: string;
  createdByUserID: string;
  requesterRole: string;
};

export type ChatDetail = ChatSummary & {
  memberCount: number;
  ownerCount: number;
};

export type ChatMember = {
  chatID: string;
  userID: string;
  role: "owner" | "member";
  joinedAt: string;
};

export type ChatMessage = {
  messageID: string;
  chatID: string;
  userID: string;
  body: string;
  createdAt: string;
  updatedAt: string;
};

export type ChatMessagesPage = {
  messages: ChatMessage[];
  nextBeforeMessageID: string | null;
};

export type PollType = "single_choice" | "multiple_choice" | "ranked_choice" | "free_text";

export type PollSummary = {
  pollID: string;
  chatID: string;
  creatorUserID: string;
  question: string;
  type: PollType;
  allowChangeVote: boolean;
  isAnonymous: boolean;
  status: "open" | "closed";
  expiresAt: string;
  createdAt: string;
  updatedAt: string;
  closedAt: string;
  optionCount: number;
  totalVotes: number;
  totalVoters: number;
  responseCount: number;
  summaryMessageID: string;
};

export type PollOption = {
  optionID: string;
  label: string;
  ord: number;
  isActive: boolean;
  voteCount: number;
};

export type PollTextResponse = {
  responseID: string;
  userID: string;
  textValue: string;
  createdAt: string;
};

export type PollDetail = PollSummary & {
  options: PollOption[];
  responses: PollTextResponse[];
};

export type PollParticipation = {
  pollID: string;
  chatID: string;
  type: PollType;
  isAnonymous: boolean;
  eligibleCount: number;
  votedCount: number;
  notVotedCount: number;
  eligibleUserIDs: string[];
  votedUserIDs: string[];
  notVotedUserIDs: string[];
};

export type CreatePollInput = {
  question: string;
  type: PollType;
  allowChangeVote: boolean;
  isAnonymous: boolean;
  expiresAt?: number;
  options?: string[];
};

export type EditPollInput = {
  question?: string;
  allowChangeVote?: boolean;
  isAnonymous?: boolean;
  status?: "open" | "closed";
  expiresAt?: number | null;
  options?: string[];
};

export type PollVotePayload = {
  optionIDs: string[];
};
