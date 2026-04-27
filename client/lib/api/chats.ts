import { request } from "./http";
import { hydrateUserIdentities } from "./identity";
import { parseChatMember, parseChatMessage, parseChatSummary, parseJsonArray } from "./parsers";
import type { ChatDetail, ChatMember, ChatMessage, ChatMessagesPage, ChatSummary } from "./types";
import TypescriptUtils from "../TypescriptUtils";

export async function listChats(beforeChatID?: string): Promise<ChatSummary[]> {
  const params = new URLSearchParams();
  params.set("limit", "20");
  if (!TypescriptUtils.isNullOrWhiteSpace(beforeChatID)) {
    params.set("beforeChatID", beforeChatID as string);
  }

  const data = await request<{ chats?: unknown }>(`/api/chats?${params.toString()}`);
  const chats = parseJsonArray(data.chats, parseChatSummary);
  await hydrateUserIdentities(chats.map((chat) => chat.createdByUserID));
  return chats;
}

export async function getChat(chatID: string): Promise<ChatDetail> {
  const data = await request<Record<string, unknown>>(`/api/chats/${chatID}`);
  const summary = parseChatSummary(data);
  if (!summary) {
    throw new Error("Invalid chat response");
  }

  await hydrateUserIdentities([summary.createdByUserID]);
  return {
    ...summary,
    memberCount: TypescriptUtils.parseInteger(data.memberCount) ?? 0,
    ownerCount: TypescriptUtils.parseInteger(data.ownerCount) ?? 0,
  };
}

export async function createChat(title: string): Promise<ChatSummary> {
  const data = await request<Record<string, unknown>>("/api/chats", {
    method: "POST",
    body: JSON.stringify({ title }),
  });

  const chatID = TypescriptUtils.parseString(data.chatID);
  if (TypescriptUtils.isNullOrWhiteSpace(chatID)) {
    throw new Error("Invalid chat creation response");
  }

  return getChat(chatID as string);
}

export async function editChat(chatID: string, title: string): Promise<void> {
  await request(`/api/chats/${chatID}`, {
    method: "PUT",
    body: JSON.stringify({ title }),
  });
}

export async function deleteChat(chatID: string): Promise<void> {
  await request(`/api/chats/${chatID}`, { method: "DELETE" });
}

export async function listChatMembers(chatID: string): Promise<ChatMember[]> {
  const data = await request<{ members?: unknown }>(`/api/chats/${chatID}/members`);
  const members = parseJsonArray(data.members, parseChatMember);
  await hydrateUserIdentities(members.map((member) => member.userID));
  return members;
}

export async function addChatMember(chatID: string, userID: string, role: "owner" | "member"): Promise<void> {
  await request(`/api/chats/${chatID}/members`, {
    method: "POST",
    body: JSON.stringify({ userID, role }),
  });
}

export async function editChatMemberRole(chatID: string, userID: string, role: "owner" | "member"): Promise<void> {
  await request(`/api/chats/${chatID}/members/${userID}`, {
    method: "PUT",
    body: JSON.stringify({ role }),
  });
}

export async function removeChatMember(chatID: string, userID: string): Promise<void> {
  await request(`/api/chats/${chatID}/members/${userID}`, { method: "DELETE" });
}

export async function getChatMessages(chatID: string, beforeMessageID?: string): Promise<ChatMessagesPage> {
  const params = new URLSearchParams();
  params.set("limit", "30");
  if (!TypescriptUtils.isNullOrWhiteSpace(beforeMessageID)) {
    params.set("beforeMessageID", beforeMessageID as string);
  }

  const data = await request<{ messages?: unknown; nextBeforeMessageID?: unknown }>(`/api/chats/${chatID}/messages?${params.toString()}`);
  const messages = parseJsonArray(data.messages, parseChatMessage);
  await hydrateUserIdentities(messages.map((message) => message.userID));

  return {
    messages,
    nextBeforeMessageID: TypescriptUtils.parseString(data.nextBeforeMessageID) ?? null,
  };
}

export async function createChatMessage(chatID: string, body: string): Promise<ChatMessage> {
  const data = await request<Record<string, unknown>>(`/api/chats/${chatID}/messages`, {
    method: "POST",
    body: JSON.stringify({ body }),
  });

  const message = parseChatMessage(data);
  if (!message) {
    throw new Error("Invalid message response");
  }

  await hydrateUserIdentities([message.userID]);
  return message;
}

export async function editChatMessage(chatID: string, messageID: string, body: string): Promise<void> {
  await request(`/api/chats/${chatID}/messages/${messageID}`, {
    method: "PUT",
    body: JSON.stringify({ body }),
  });
}

export async function deleteChatMessage(chatID: string, messageID: string): Promise<void> {
  await request(`/api/chats/${chatID}/messages/${messageID}`, { method: "DELETE" });
}
