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
  getPoll,
  listChatMembers,
  listChatPolls,
  PollOption,
  PollSummary,
  removeChatMember,
  submitPollTextResponse,
} from "../../../lib/api";

type UseChatDetailStateResult = {
  chat: ChatDetail | null;
  messages: ChatMessage[];
  nextBeforeMessageID: string | null;
  members: ChatMember[];
  polls: PollSummary[];
  pollOptionsByPollID: Record<string, PollOption[]>;
  freeTextResponseByPollID: Record<string, string>;
  savingFreeTextByPollID: Record<string, boolean>;
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
  setFreeTextResponseDraft: (pollID: string, value: string) => void;
  setEditMessageID: (value: string | null) => void;
  setEditMessageBody: (value: string) => void;
  setNewChatTitle: (value: string) => void;
  setMemberUserIDDraft: (value: string) => void;
  loadAll: () => Promise<void>;
  createOrEditMessage: () => Promise<void>;
  autosaveFreeTextResponse: (pollID: string, force?: boolean) => Promise<void>;
  saveEditedMessage: () => Promise<void>;
  removeMessage: (messageID: string) => void;
  loadOlderMessages: () => Promise<void>;
  updateChatTitle: () => Promise<void>;
  addMember: () => Promise<void>;
  toggleMemberRole: (member: ChatMember) => Promise<void>;
  removeMember: (member: ChatMember) => void;
  removeChat: () => void;
};

export function useChatDetailState(chatID: string, currentUserID: string): UseChatDetailStateResult {
  const router = useRouter();

  const [chat, setChat] = useState<ChatDetail | null>(null);
  const [messages, setMessages] = useState<ChatMessage[]>([]);
  const [nextBeforeMessageID, setNextBeforeMessageID] = useState<string | null>(null);
  const [members, setMembers] = useState<ChatMember[]>([]);
  const [polls, setPolls] = useState<PollSummary[]>([]);
  const [pollOptionsByPollID, setPollOptionsByPollID] = useState<Record<string, PollOption[]>>({});
  const [freeTextResponseByPollID, setFreeTextResponseByPollID] = useState<Record<string, string>>({});
  const [lastSavedFreeTextByPollID, setLastSavedFreeTextByPollID] = useState<Record<string, string>>({});
  const [savingFreeTextByPollID, setSavingFreeTextByPollID] = useState<Record<string, boolean>>({});

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

      // Poll list payload does not include option labels, so hydrate details once per visible poll.
      if (pollData.length > 0) {
        const hydratedEntries = await Promise.all(
          pollData.map(async (poll) => {
            try {
              const detail = await getPoll(poll.pollID);
              const ownFreeText = detail.type === "free_text"
                ? (detail.responses.find((response) => response.userID === currentUserID)?.textValue ?? "")
                : "";
              return [poll.pollID, detail.options as PollOption[], ownFreeText] as const;
            } catch {
              return [poll.pollID, [] as PollOption[], ""] as const;
            }
          })
        );
        setPollOptionsByPollID(Object.fromEntries(hydratedEntries.map(([pollID, options]) => [pollID, options])));

        const nextSaved = Object.fromEntries(
          hydratedEntries
            .filter(([pollID]) => pollData.some((poll) => poll.pollID === pollID && poll.type === "free_text"))
            .map(([pollID, _options, ownFreeText]) => [pollID, ownFreeText.trim()])
        );
        setLastSavedFreeTextByPollID(nextSaved);
        setFreeTextResponseByPollID((prev) => {
          const merged: Record<string, string> = {};
          for (const [pollID, _options, ownFreeText] of hydratedEntries) {
            const localDraft = prev[pollID];
            merged[pollID] = typeof localDraft === "string" ? localDraft : ownFreeText;
          }
          return merged;
        });
      } else {
        setPollOptionsByPollID({});
        setFreeTextResponseByPollID({});
        setLastSavedFreeTextByPollID({});
        setSavingFreeTextByPollID({});
      }

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
  }, [chatID, currentUserID]);

  const setFreeTextResponseDraft = useCallback((pollID: string, value: string) => {
    setFreeTextResponseByPollID((prev) => ({ ...prev, [pollID]: value }));
  }, []);

  const autosaveFreeTextResponse = useCallback(async (pollID: string, force = false) => {
    const normalized = (freeTextResponseByPollID[pollID] ?? "").trim();
    if (!normalized) {
      return;
    }
    if (!force && normalized === (lastSavedFreeTextByPollID[pollID] ?? "")) {
      return;
    }

    setSavingFreeTextByPollID((prev) => ({ ...prev, [pollID]: true }));
    try {
      await submitPollTextResponse(pollID, normalized);
      setLastSavedFreeTextByPollID((prev) => ({ ...prev, [pollID]: normalized }));
    } catch (e: any) {
      Alert.alert("Auto-Save Failed", e?.message || "Please retry.");
      await loadAll();
    } finally {
      setSavingFreeTextByPollID((prev) => ({ ...prev, [pollID]: false }));
    }
  }, [freeTextResponseByPollID, lastSavedFreeTextByPollID, loadAll]);

  const createOrEditMessage = useCallback(async () => {
    if (busy || !chatID || !messageDraft.trim()) {
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
  }, [busy, chatID, loadAll, messageDraft]);

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
    pollOptionsByPollID,
    freeTextResponseByPollID,
    savingFreeTextByPollID,
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
    setFreeTextResponseDraft,
    setEditMessageID,
    setEditMessageBody,
    setNewChatTitle,
    setMemberUserIDDraft,
    loadAll,
    createOrEditMessage,
    autosaveFreeTextResponse,
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
