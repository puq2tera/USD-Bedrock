import { useCallback, useMemo, useState } from "react";
import { Alert } from "react-native";
import { useRouter } from "expo-router";
import {
  addChatMember,
  canManageChat,
  ChatDetail,
  ChatMember,
  ChatMessage,
  createChatMessage,
  deleteChat,
  deleteChatMessage,
  editChat,
  editChatMemberRole,
  editChatMessage,
  getChat,
  getChatMessages,
  listChatMembers,
  listChatPolls,
  PollSummary,
  removeChatMember,
} from "../../../lib/api";

type UseChatDetailStateResult = {
  chat: ChatDetail | null;
  messages: ChatMessage[];
  nextBeforeMessageID: string | null;
  members: ChatMember[];
  polls: PollSummary[];
  loading: boolean;
  refreshing: boolean;
  loadingMoreMessages: boolean;
  busy: boolean;
  error: string | null;
  messageDraft: string;
  editMessageID: string | null;
  editMessageBody: string;
  newChatTitle: string;
  memberUserIDDraft: string;
  isOwner: boolean;
  setRefreshing: (value: boolean) => void;
  setMessageDraft: (value: string) => void;
  setEditMessageID: (value: string | null) => void;
  setEditMessageBody: (value: string) => void;
  setNewChatTitle: (value: string) => void;
  setMemberUserIDDraft: (value: string) => void;
  loadAll: () => Promise<void>;
  createOrEditMessage: () => Promise<void>;
  saveEditedMessage: () => Promise<void>;
  removeMessage: (messageID: string) => void;
  loadOlderMessages: () => Promise<void>;
  updateChatTitle: () => Promise<void>;
  addMember: () => Promise<void>;
  toggleMemberRole: (member: ChatMember) => Promise<void>;
  removeMember: (member: ChatMember) => void;
  removeChat: () => void;
};

export function useChatDetailState(chatID: string): UseChatDetailStateResult {
  const router = useRouter();

  const [chat, setChat] = useState<ChatDetail | null>(null);
  const [messages, setMessages] = useState<ChatMessage[]>([]);
  const [nextBeforeMessageID, setNextBeforeMessageID] = useState<string | null>(null);
  const [members, setMembers] = useState<ChatMember[]>([]);
  const [polls, setPolls] = useState<PollSummary[]>([]);

  const [loading, setLoading] = useState(true);
  const [refreshing, setRefreshing] = useState(false);
  const [loadingMoreMessages, setLoadingMoreMessages] = useState(false);
  const [busy, setBusy] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const [messageDraft, setMessageDraft] = useState("");
  const [editMessageID, setEditMessageID] = useState<string | null>(null);
  const [editMessageBody, setEditMessageBody] = useState("");

  const [newChatTitle, setNewChatTitle] = useState("");
  const [memberUserIDDraft, setMemberUserIDDraft] = useState("");

  const isOwner = useMemo(() => (chat ? canManageChat(chat) : false), [chat]);

  const loadAll = useCallback(async () => {
    if (!chatID) {
      setError("Invalid chat ID");
      setLoading(false);
      setRefreshing(false);
      return;
    }

    try {
      setError(null);
      const [chatData, messagePage, pollData] = await Promise.all([
        getChat(chatID),
        getChatMessages(chatID),
        listChatPolls(chatID),
      ]);

      setChat(chatData);
      setNewChatTitle(chatData.title);
      setMessages(messagePage.messages);
      setNextBeforeMessageID(messagePage.nextBeforeMessageID);
      setPolls(pollData);

      if (canManageChat(chatData)) {
        setMembers(await listChatMembers(chatID));
      } else {
        setMembers([]);
      }
    } catch (e: any) {
      setError(e?.message || "Failed to load chat");
    } finally {
      setLoading(false);
      setRefreshing(false);
    }
  }, [chatID]);

  const createOrEditMessage = useCallback(async () => {
    if (!chatID || !messageDraft.trim()) {
      return;
    }

    setBusy(true);
    try {
      await createChatMessage(chatID, messageDraft.trim());
      setMessageDraft("");
      await loadAll();
    } catch (e: any) {
      Alert.alert("Message not sent", e?.message || "Please retry.");
    } finally {
      setBusy(false);
    }
  }, [chatID, loadAll, messageDraft]);

  const saveEditedMessage = useCallback(async () => {
    if (!chatID || !editMessageID || !editMessageBody.trim()) {
      return;
    }

    setBusy(true);
    try {
      await editChatMessage(chatID, editMessageID, editMessageBody.trim());
      setEditMessageID(null);
      setEditMessageBody("");
      await loadAll();
    } catch (e: any) {
      Alert.alert("Message update failed", e?.message || "Please retry.");
    } finally {
      setBusy(false);
    }
  }, [chatID, editMessageBody, editMessageID, loadAll]);

  const removeMessage = useCallback((messageID: string) => {
    if (!chatID) {
      return;
    }

    Alert.alert("Delete message", "Delete this message?", [
      { text: "Cancel", style: "cancel" },
      {
        text: "Delete",
        style: "destructive",
        onPress: async () => {
          setBusy(true);
          try {
            await deleteChatMessage(chatID, messageID);
            await loadAll();
          } catch (e: any) {
            Alert.alert("Delete failed", e?.message || "Please retry.");
          } finally {
            setBusy(false);
          }
        },
      },
    ]);
  }, [chatID, loadAll]);

  const loadOlderMessages = useCallback(async () => {
    if (!chatID || !nextBeforeMessageID || loadingMoreMessages) {
      return;
    }

    setLoadingMoreMessages(true);
    try {
      const page = await getChatMessages(chatID, nextBeforeMessageID);
      setMessages((prev) => [...prev, ...page.messages]);
      setNextBeforeMessageID(page.nextBeforeMessageID);
    } catch (e: any) {
      Alert.alert("Could not load older messages", e?.message || "Please retry.");
    } finally {
      setLoadingMoreMessages(false);
    }
  }, [chatID, loadingMoreMessages, nextBeforeMessageID]);

  const updateChatTitle = useCallback(async () => {
    if (!chatID || !newChatTitle.trim()) {
      return;
    }

    setBusy(true);
    try {
      await editChat(chatID, newChatTitle.trim());
      await loadAll();
    } catch (e: any) {
      Alert.alert("Chat update failed", e?.message || "Please retry.");
    } finally {
      setBusy(false);
    }
  }, [chatID, loadAll, newChatTitle]);

  const addMember = useCallback(async () => {
    if (!chatID || !memberUserIDDraft.trim()) {
      Alert.alert("Missing user", "Enter a user ID.");
      return;
    }

    setBusy(true);
    try {
      await addChatMember(chatID, memberUserIDDraft.trim(), "member");
      setMemberUserIDDraft("");
      await loadAll();
    } catch (e: any) {
      Alert.alert("Member add failed", e?.message || "Please retry.");
    } finally {
      setBusy(false);
    }
  }, [chatID, loadAll, memberUserIDDraft]);

  const toggleMemberRole = useCallback(async (member: ChatMember) => {
    if (!chatID) {
      return;
    }

    const nextRole = member.role === "owner" ? "member" : "owner";
    setBusy(true);
    try {
      await editChatMemberRole(chatID, member.userID, nextRole);
      await loadAll();
    } catch (e: any) {
      Alert.alert("Role change failed", e?.message || "Please retry.");
    } finally {
      setBusy(false);
    }
  }, [chatID, loadAll]);

  const removeMember = useCallback((member: ChatMember) => {
    if (!chatID) {
      return;
    }

    Alert.alert("Remove member", "Remove this member from this chat?", [
      { text: "Cancel", style: "cancel" },
      {
        text: "Continue",
        style: "destructive",
        onPress: () => {
          Alert.alert("Confirm removal", "This action cannot be undone.", [
            { text: "Cancel", style: "cancel" },
            {
              text: "Remove",
              style: "destructive",
              onPress: async () => {
                setBusy(true);
                try {
                  await removeChatMember(chatID, member.userID);
                  await loadAll();
                } catch (e: any) {
                  Alert.alert("Member removal failed", e?.message || "Please retry.");
                } finally {
                  setBusy(false);
                }
              },
            },
          ]);
        },
      },
    ]);
  }, [chatID, loadAll]);

  const removeChat = useCallback(() => {
    if (!chatID) {
      return;
    }

    Alert.alert("Delete chat", "Delete this chat and all of its messages and polls?", [
      { text: "Cancel", style: "cancel" },
      {
        text: "Continue",
        style: "destructive",
        onPress: () => {
          Alert.alert("Final confirmation", "This action is permanent.", [
            { text: "Cancel", style: "cancel" },
            {
              text: "Delete chat",
              style: "destructive",
              onPress: async () => {
                setBusy(true);
                try {
                  await deleteChat(chatID);
                  router.replace("/");
                } catch (e: any) {
                  Alert.alert("Chat delete failed", e?.message || "Please retry.");
                } finally {
                  setBusy(false);
                }
              },
            },
          ]);
        },
      },
    ]);
  }, [chatID, router]);

  return {
    chat,
    messages,
    nextBeforeMessageID,
    members,
    polls,
    loading,
    refreshing,
    loadingMoreMessages,
    busy,
    error,
    messageDraft,
    editMessageID,
    editMessageBody,
    newChatTitle,
    memberUserIDDraft,
    isOwner,
    setRefreshing,
    setMessageDraft,
    setEditMessageID,
    setEditMessageBody,
    setNewChatTitle,
    setMemberUserIDDraft,
    loadAll,
    createOrEditMessage,
    saveEditedMessage,
    removeMessage,
    loadOlderMessages,
    updateChatTitle,
    addMember,
    toggleMemberRole,
    removeMember,
    removeChat,
  };
}
