import { useCallback, useMemo, useState } from "react";
import { Alert } from "react-native";
import { useRouter } from "expo-router";
import {
  canManagePoll,
  deleteAllPollVotes,
  deletePoll,
  deletePollVotes,
  EditPollInput,
  editPoll,
  getPoll,
  getPollParticipation,
  PollDetail,
  PollOption,
  PollParticipation,
  submitPollTextResponse,
  submitPollVotes,
} from "../../../../../lib/api";

type UsePollDetailStateResult = {
  poll: PollDetail | null;
  participation: PollParticipation | null;
  loading: boolean;
  refreshing: boolean;
  busy: boolean;
  error: string | null;
  selectedOptionIDs: string[];
  textResponse: string;
  savingTextResponse: boolean;
  editing: boolean;
  editQuestion: string;
  editAllowChangeVote: boolean;
  editIsAnonymous: boolean;
  editExpiresAt: string;
  editOptions: string[];
  isCreator: boolean;
  setRefreshing: (value: boolean) => void;
  setTextResponse: (value: string) => void;
  setEditing: (value: boolean) => void;
  setEditQuestion: (value: string) => void;
  setEditAllowChangeVote: (value: boolean) => void;
  setEditIsAnonymous: (value: boolean) => void;
  setEditExpiresAt: (value: string) => void;
  setEditOptions: (value: string[]) => void;
  loadPoll: () => Promise<void>;
  toggleOptionSelection: (option: PollOption) => Promise<void>;
  autosaveTextResponse: (force?: boolean) => Promise<void>;
  removeParticipation: () => Promise<void>;
  removeAllParticipations: () => Promise<void>;
  submitPollEdit: (overrides?: Partial<EditPollInput>) => Promise<void>;
  confirmReplaceOptions: () => void;
  transitionStatus: (nextStatus: "open" | "closed") => void;
  removePoll: () => void;
};

export function usePollDetailState(chatID: string, resolvedPollID: string, currentUserID: string): UsePollDetailStateResult {
  const router = useRouter();

  const [poll, setPoll] = useState<PollDetail | null>(null);
  const [participation, setParticipation] = useState<PollParticipation | null>(null);
  const [loading, setLoading] = useState(true);
  const [refreshing, setRefreshing] = useState(false);
  const [busy, setBusy] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const [selectedOptionIDs, setSelectedOptionIDs] = useState<string[]>([]);
  const [textResponse, setTextResponse] = useState("");
  const [lastSavedTextResponse, setLastSavedTextResponse] = useState("");
  const [savingTextResponse, setSavingTextResponse] = useState(false);

  const [editing, setEditing] = useState(false);
  const [editQuestion, setEditQuestion] = useState("");
  const [editAllowChangeVote, setEditAllowChangeVote] = useState(true);
  const [editIsAnonymous, setEditIsAnonymous] = useState(false);
  const [editExpiresAt, setEditExpiresAt] = useState("");
  const [editOptions, setEditOptions] = useState<string[]>([]);

  const isCreator = useMemo(() => (poll ? canManagePoll(poll) : false), [poll]);

  const loadPoll = useCallback(async () => {
    if (!resolvedPollID) {
      setError("Invalid poll ID");
      setLoading(false);
      setRefreshing(false);
      return;
    }

    try {
      setError(null);
      const pollData = await getPoll(resolvedPollID);
      setPoll(pollData);
      setEditQuestion(pollData.question);
      setEditAllowChangeVote(pollData.allowChangeVote);
      setEditIsAnonymous(pollData.isAnonymous);
      setEditExpiresAt(pollData.expiresAt ? new Date(Number(pollData.expiresAt) * 1000).toISOString().slice(0, 16) : "");
      setEditOptions(pollData.options.map((option) => option.label));

      if (pollData.type === "free_text") {
        // Free-text payloads include all responses. Keep only the current user's response in edit state.
        const ownResponse = pollData.responses.find((response) => response.userID === currentUserID);
        const normalizedExisting = ownResponse?.textValue ?? "";
        setTextResponse(normalizedExisting);
        setLastSavedTextResponse(normalizedExisting.trim());
      }

      const participationData = await getPollParticipation(resolvedPollID);
      setParticipation(participationData);
    } catch (e: any) {
      setError(e?.message || "Failed to load poll");
    } finally {
      setLoading(false);
      setRefreshing(false);
    }
  }, [currentUserID, resolvedPollID]);

  const toggleOptionSelection = useCallback(async (option: PollOption) => {
    if (!poll || poll.status !== "open") {
      return;
    }

    let nextSelectedOptionIDs: string[] = [];
    if (poll.type === "single_choice") {
      nextSelectedOptionIDs = selectedOptionIDs.includes(option.optionID) ? [] : [option.optionID];
    } else if (poll.type === "ranked_choice") {
      // Ranked-choice preserves tap order so submission order maps to rank order.
      if (selectedOptionIDs.includes(option.optionID)) {
        nextSelectedOptionIDs = selectedOptionIDs.filter((optionID) => optionID !== option.optionID);
      } else {
        nextSelectedOptionIDs = [...selectedOptionIDs, option.optionID];
      }
    } else {
      nextSelectedOptionIDs = selectedOptionIDs.includes(option.optionID)
        ? selectedOptionIDs.filter((optionID) => optionID !== option.optionID)
        : [...selectedOptionIDs, option.optionID];
    }

    setBusy(true);
    try {
      if (nextSelectedOptionIDs.length < 1) {
        await deletePollVotes(poll.pollID);
      } else {
        await submitPollVotes(poll.pollID, { optionIDs: nextSelectedOptionIDs });
      }
      setSelectedOptionIDs(nextSelectedOptionIDs);
      await loadPoll();
    } catch (e: any) {
      Alert.alert("Vote failed", e?.message || "Refreshing poll state.");
      await loadPoll();
    } finally {
      setBusy(false);
    }
  }, [loadPoll, poll, selectedOptionIDs]);

  const autosaveTextResponse = useCallback(async (force = false) => {
    if (!poll || poll.type !== "free_text") {
      return;
    }

    const normalized = textResponse.trim();
    if (!normalized) {
      return;
    }
    if (!force && normalized === lastSavedTextResponse) {
      return;
    }

    setSavingTextResponse(true);
    setBusy(true);
    try {
      await submitPollTextResponse(poll.pollID, normalized);
      setLastSavedTextResponse(normalized);
    } catch (e: any) {
      Alert.alert("Auto-Save Failed", e?.message || "Refreshing poll state.");
      await loadPoll();
    } finally {
      setSavingTextResponse(false);
      setBusy(false);
    }
  }, [lastSavedTextResponse, loadPoll, poll, textResponse]);

  const removeParticipation = useCallback(async () => {
    if (!poll) {
      return;
    }

    setBusy(true);
    try {
      await deletePollVotes(poll.pollID);
      setSelectedOptionIDs([]);
      setTextResponse("");
      setLastSavedTextResponse("");
      await loadPoll();
    } catch (e: any) {
      Alert.alert("Remove failed", e?.message || "Refreshing poll state.");
      await loadPoll();
    } finally {
      setBusy(false);
    }
  }, [loadPoll, poll]);

  const removeAllParticipations = useCallback(async () => {
    if (!poll) {
      return;
    }

    setBusy(true);
    try {
      await deleteAllPollVotes(poll.pollID);
      setSelectedOptionIDs([]);
      setTextResponse("");
      setLastSavedTextResponse("");
      await loadPoll();
    } catch (e: any) {
      Alert.alert("Reset failed", e?.message || "Refreshing poll state.");
      await loadPoll();
    } finally {
      setBusy(false);
    }
  }, [loadPoll, poll]);

  const submitPollEdit = useCallback(async (overrides?: Partial<EditPollInput>) => {
    if (!poll) {
      return;
    }

    const payload: EditPollInput = {
      question: editQuestion.trim(),
      allowChangeVote: editAllowChangeVote,
      isAnonymous: editIsAnonymous,
      expiresAt: editExpiresAt.trim() ? Math.floor(new Date(editExpiresAt.trim()).getTime() / 1000) : null,
      ...(poll.type !== "free_text" ? { options: editOptions.map((option) => option.trim()).filter((option) => option.length > 0) } : {}),
      ...overrides,
    };

    if (!payload.question) {
      Alert.alert("Missing question", "Question is required.");
      return;
    }

    if (poll.type !== "free_text" && (payload.options?.length ?? 0) < 2) {
      Alert.alert("Invalid options", "Choice polls need at least two options.");
      return;
    }

    setBusy(true);
    try {
      await editPoll(poll.pollID, payload);
      setEditing(false);
      await loadPoll();
    } catch (e: any) {
      Alert.alert("Poll update failed", e?.message || "Please retry.");
    } finally {
      setBusy(false);
    }
  }, [editAllowChangeVote, editExpiresAt, editIsAnonymous, editOptions, editQuestion, loadPoll, poll]);

  const confirmReplaceOptions = useCallback(() => {
    Alert.alert("Replace options", "Replacing options resets existing votes. Continue?", [
      { text: "Cancel", style: "cancel" },
      {
        text: "Continue",
        style: "destructive",
        onPress: () => void submitPollEdit(),
      },
    ]);
  }, [submitPollEdit]);

  const transitionStatus = useCallback((nextStatus: "open" | "closed") => {
    Alert.alert(
      nextStatus === "closed" ? "Close poll" : "Reopen poll",
      nextStatus === "closed" ? "Close this poll now?" : "Reopen this poll?",
      [
        { text: "Cancel", style: "cancel" },
        {
          text: nextStatus === "closed" ? "Close" : "Reopen",
          style: nextStatus === "closed" ? "destructive" : "default",
          onPress: () => void submitPollEdit({ status: nextStatus }),
        },
      ]
    );
  }, [submitPollEdit]);

  const removePoll = useCallback(() => {
    if (!poll) {
      return;
    }

    Alert.alert("Delete poll", "Delete this poll permanently?", [
      { text: "Cancel", style: "cancel" },
      {
        text: "Continue",
        style: "destructive",
        onPress: () => {
          Alert.alert("Final confirmation", "This action is permanent and cannot be undone.", [
            { text: "Cancel", style: "cancel" },
            {
              text: "Delete poll",
              style: "destructive",
              onPress: async () => {
                setBusy(true);
                try {
                  await deletePoll(poll.pollID);
                  router.replace(`/chat/${chatID}`);
                } catch (e: any) {
                  Alert.alert("Delete failed", e?.message || "Please retry.");
                } finally {
                  setBusy(false);
                }
              },
            },
          ]);
        },
      },
    ]);
  }, [chatID, poll, router]);

  return {
    poll,
    participation,
    loading,
    refreshing,
    busy,
    error,
    selectedOptionIDs,
    textResponse,
    savingTextResponse,
    editing,
    editQuestion,
    editAllowChangeVote,
    editIsAnonymous,
    editExpiresAt,
    editOptions,
    isCreator,
    setRefreshing,
    setTextResponse,
    setEditing,
    setEditQuestion,
    setEditAllowChangeVote,
    setEditIsAnonymous,
    setEditExpiresAt,
    setEditOptions,
    loadPoll,
    toggleOptionSelection,
    autosaveTextResponse,
    removeParticipation,
    removeAllParticipations,
    submitPollEdit,
    confirmReplaceOptions,
    transitionStatus,
    removePoll,
  };
}
