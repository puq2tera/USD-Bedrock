import { useCallback, useEffect, useLayoutEffect, useRef, useState } from "react";
import {
  ActivityIndicator,
  Alert,
  KeyboardAvoidingView,
  Platform,
  RefreshControl,
  StyleSheet,
  Text,
  TextInput,
  TouchableOpacity,
  View,
} from "react-native";
import { Redirect, useFocusEffect, useLocalSearchParams, useNavigation, useRouter } from "expo-router";
import { KeyboardAwareScrollView } from "../../../components/KeyboardAwareScrollView";
import TypescriptUtils from "../../../lib/TypescriptUtils";
import { deletePollVotes, getIdentityLabel, PollType, submitPollVotes } from "../../../lib/api";
import { useAuth } from "../../../lib/auth";
import { formatDateTimeForDisplay, parseDateTime } from "../../../lib/dateTime";
import { appColors, commonStyles } from "../../../lib/styles";
import { useChatDetailState } from "./useChatDetailState";

export function ChatDetailScreen() {
  const { isAuthenticated, user } = useAuth();
  const router = useRouter();
  const navigation = useNavigation();
  const { id } = useLocalSearchParams<{ id: string }>();
  const chatID = TypescriptUtils.parseString(id) ?? "";

  const state = useChatDetailState(chatID, user?.userID ?? "");
  const [inlineSelectedOptionIDs, setInlineSelectedOptionIDs] = useState<Record<string, string[]>>({});
  const [inlineVotingPollIDs, setInlineVotingPollIDs] = useState<Record<string, boolean>>({});
  const [showExactTimestamps, setShowExactTimestamps] = useState(false);
  const inlineFreeTextDebounceRef = useRef<Record<string, ReturnType<typeof setTimeout>>>({});

  useEffect(() => () => {
    Object.values(inlineFreeTextDebounceRef.current).forEach((timer) => clearTimeout(timer));
  }, []);

  useLayoutEffect(() => {
    navigation.setOptions({
      headerTitle: state.chat?.title || "Chat",
      headerRight: () => (
        <View style={styles.headerActions}>
          <TouchableOpacity style={commonStyles.circularIconButton} onPress={() => router.push(`/chat/${chatID}/settings`)}>
            <Text style={commonStyles.circularIconButtonText}>⚙</Text>
          </TouchableOpacity>
          <TouchableOpacity style={commonStyles.circularIconButton} onPress={() => router.push("/account")}>
            <Text style={commonStyles.circularIconButtonText}>👤</Text>
          </TouchableOpacity>
        </View>
      ),
    });
  }, [chatID, navigation, router, state.chat?.title]);

  useFocusEffect(
    useCallback(() => {
      void state.loadAll();
    }, [state.loadAll])
  );

  if (!isAuthenticated) {
    return <Redirect href="/login" />;
  }

  if (state.loading) {
    return (
      <View style={commonStyles.centeredScreen}>
        <ActivityIndicator color={appColors.accent} size="large" />
      </View>
    );
  }

  if (!state.chat || state.error) {
    return (
      <View style={commonStyles.centeredScreen}>
        <Text style={commonStyles.errorText}>{state.error || "Chat not found"}</Text>
        <TouchableOpacity style={commonStyles.retryButton} onPress={() => void state.loadAll()}>
          <Text style={commonStyles.retryButtonText}>Retry</Text>
        </TouchableOpacity>
      </View>
    );
  }

  const timeline = [...state.messages.map((message, index) => ({
    type: "message" as const,
    createdAt: message.createdAt,
    key: `message-${message.messageID}-${index}`,
    message,
  })), ...state.polls.map((poll, index) => ({
    type: "poll" as const,
    createdAt: poll.createdAt,
    key: `poll-${poll.pollID}-${index}`,
    poll,
  }))].sort((left, right) => {
    const leftSeconds = Math.floor((parseDateTime(left.createdAt)?.getTime() ?? 0) / 1000);
    const rightSeconds = Math.floor((parseDateTime(right.createdAt)?.getTime() ?? 0) / 1000);
    return leftSeconds - rightSeconds;
  });

  const getEntryDate = (createdAt: string): Date | null => parseDateTime(createdAt);

  const getRelativeGapMinutes = (previousDate: Date | null, currentDate: Date | null): number => {
    if (!previousDate || !currentDate) {
      return Number.POSITIVE_INFINITY;
    }
    return Math.max(0, Math.floor((currentDate.getTime() - previousDate.getTime()) / 60000));
  };

  const isSameLocalDay = (left: Date, right: Date): boolean => (
    left.getFullYear() === right.getFullYear()
    && left.getMonth() === right.getMonth()
    && left.getDate() === right.getDate()
  );

  const shouldShowTimeSeparator = (index: number): boolean => {
    if (index < 1) {
      return true;
    }

    const currentDate = getEntryDate(timeline[index].createdAt);
    const previousDate = getEntryDate(timeline[index - 1].createdAt);
    if (!currentDate || !previousDate) {
      return false;
    }

    if (!isSameLocalDay(previousDate, currentDate)) {
      return true;
    }

    // 30+ minute silence is treated as a new conversational burst.
    return getRelativeGapMinutes(previousDate, currentDate) >= 30;
  };

  const formatTimelineSeparator = (createdAt: string): string => {
    const date = getEntryDate(createdAt);
    if (!date) {
      return "";
    }

    const now = new Date();
    const todayStart = new Date(now.getFullYear(), now.getMonth(), now.getDate());
    const targetStart = new Date(date.getFullYear(), date.getMonth(), date.getDate());
    const dayDifference = Math.round((todayStart.getTime() - targetStart.getTime()) / 86400000);

    const timeOnly = new Intl.DateTimeFormat(undefined, { hour: "numeric", minute: "2-digit" }).format(date);
    if (dayDifference === 0) {
      return timeOnly;
    }

    if (dayDifference === 1) {
      return `Yesterday, ${timeOnly}`;
    }

    if (dayDifference > 1 && dayDifference < 7) {
      const weekdayWithTime = new Intl.DateTimeFormat(undefined, {
        weekday: "long",
        hour: "numeric",
        minute: "2-digit",
      }).format(date);
      return weekdayWithTime;
    }

    if (date.getFullYear() === now.getFullYear()) {
      return new Intl.DateTimeFormat(undefined, {
        month: "short",
        day: "numeric",
        hour: "numeric",
        minute: "2-digit",
      }).format(date);
    }

    return new Intl.DateTimeFormat(undefined, {
      month: "short",
      day: "numeric",
      year: "numeric",
      hour: "numeric",
      minute: "2-digit",
    }).format(date);
  };

  const formatExactTimestamp = (createdAt: string): string => {
    const date = getEntryDate(createdAt);
    if (!date) {
      return "";
    }
    return new Intl.DateTimeFormat(undefined, {
      weekday: "short",
      month: "short",
      day: "numeric",
      year: "numeric",
      hour: "numeric",
      minute: "2-digit",
    }).format(date);
  };

  const getPollStatusChip = (expiresAt: string, status: "open" | "closed"): string => {
    const expiresDate = parseDateTime(expiresAt);
    if (expiresDate) {
      return `Expires ${formatDateTimeForDisplay(expiresAt)}`;
    }
    return status === "open" ? "Active" : "Closed";
  };

  const handleInlineOptionPress = async (pollID: string, pollType: string, optionID: string) => {
    if (inlineVotingPollIDs[pollID]) {
      return;
    }

    const current = inlineSelectedOptionIDs[pollID] ?? [];
    let next: string[] = [];

    if (pollType === "single_choice") {
      next = current.includes(optionID) ? [] : [optionID];
    } else if (pollType === "ranked_choice") {
      // Ranked-choice inline taps preserve order so option index maps to rank.
      next = current.includes(optionID)
        ? current.filter((selected) => selected !== optionID)
        : [...current, optionID];
    } else {
      next = current.includes(optionID)
        ? current.filter((selected) => selected !== optionID)
        : [...current, optionID];
    }

    setInlineSelectedOptionIDs((prev) => ({ ...prev, [pollID]: next }));
    setInlineVotingPollIDs((prev) => ({ ...prev, [pollID]: true }));
    try {
      if (next.length < 1) {
        await deletePollVotes(pollID);
      } else {
        await submitPollVotes(pollID, { optionIDs: next });
      }
      await state.loadAll();
    } catch (e: any) {
      Alert.alert("Vote Failed", e?.message || "Refreshing poll state.");
      await state.loadAll();
    } finally {
      setInlineVotingPollIDs((prev) => ({ ...prev, [pollID]: false }));
    }
  };

  const getInlineRank = (pollID: string, optionID: string): number => (inlineSelectedOptionIDs[pollID] ?? []).indexOf(optionID) + 1;
  const getInlineSelected = (pollID: string, optionID: string): boolean => (inlineSelectedOptionIDs[pollID] ?? []).includes(optionID);
  const scheduleInlineFreeTextAutosave = (pollID: string) => {
    if (inlineFreeTextDebounceRef.current[pollID]) {
      clearTimeout(inlineFreeTextDebounceRef.current[pollID]);
    }
    inlineFreeTextDebounceRef.current[pollID] = setTimeout(() => {
      void state.autosaveFreeTextResponse(pollID, false);
    }, 1200);
  };

  return (
    <KeyboardAvoidingView style={commonStyles.screen} behavior={Platform.OS === "ios" ? "padding" : undefined}>
      <KeyboardAwareScrollView
        style={styles.thread}
        contentContainerStyle={styles.threadContent}
        onScroll={({ nativeEvent }) => {
          if (nativeEvent.contentOffset.y <= 80) {
            void state.loadOlderMessages();
          }
        }}
        onScrollBeginDrag={() => setShowExactTimestamps(true)}
        onScrollEndDrag={() => setShowExactTimestamps(false)}
        onMomentumScrollEnd={() => setShowExactTimestamps(false)}
        scrollEventThrottle={120}
        refreshControl={<RefreshControl refreshing={state.refreshing} onRefresh={() => {
          state.setRefreshing(true);
          void state.loadAll();
        }} />}
      >
        {state.loadingMoreMessages && (
          <View style={styles.loadingOlderRow}>
            <ActivityIndicator color={appColors.accent} size="small" />
            <Text style={styles.loadingOlderText}>Loading older messages…</Text>
          </View>
        )}

        {timeline.length < 1 && (
          <Text style={styles.emptyHint}>No messages yet. Start the conversation below.</Text>
        )}

        {timeline.map((entry, index) => {
          const previousDate = index > 0 ? getEntryDate(timeline[index - 1].createdAt) : null;
          const currentDate = getEntryDate(entry.createdAt);
          const gapMinutes = getRelativeGapMinutes(previousDate, currentDate);
          const burstSpacingStyle = gapMinutes >= 30
            ? styles.timelineGapLarge
            : (gapMinutes >= 8 ? styles.timelineGapMedium : styles.timelineGapTight);

          if (entry.type === "message") {
            const isOwnMessage = entry.message.userID === user?.userID;
            return (
              <View key={entry.key}>
                {shouldShowTimeSeparator(index) && (
                  <View style={styles.timeSeparatorRow}>
                    <Text style={styles.timeSeparatorText}>{formatTimelineSeparator(entry.createdAt)}</Text>
                  </View>
                )}
                <View style={[styles.timelineEntry, burstSpacingStyle, styles.messageRow, isOwnMessage ? styles.messageRowRight : styles.messageRowLeft]}>
                  <Text style={styles.messageMeta}>{isOwnMessage ? "You" : getIdentityLabel(entry.message.userID)}</Text>
                  <View style={[styles.bubble, isOwnMessage ? styles.bubbleOwn : styles.bubbleOther]}>
                    <Text style={styles.bubbleText}>{entry.message.body}</Text>
                  </View>
                  {showExactTimestamps && (
                    <Text style={[styles.exactTimestampText, isOwnMessage ? styles.exactTimestampRight : styles.exactTimestampLeft]}>
                      {formatExactTimestamp(entry.createdAt)}
                    </Text>
                  )}
                </View>
              </View>
            );
          }

          return (
            <View key={entry.key}>
              {shouldShowTimeSeparator(index) && (
                <View style={styles.timeSeparatorRow}>
                  <Text style={styles.timeSeparatorText}>{formatTimelineSeparator(entry.createdAt)}</Text>
                </View>
              )}
              <TouchableOpacity style={[styles.timelineEntry, burstSpacingStyle, styles.pollCard]} onPress={() => router.push(`/chat/${chatID}/poll/${entry.poll.pollID}`)}>
                <Text style={styles.pollTag}>Poll</Text>
                <Text style={styles.pollQuestion}>{entry.poll.question}</Text>
                {entry.poll.type !== "free_text" && (
                  <View style={styles.pollOptionsList}>
                    {entry.poll.type === "ranked_choice" && (
                      <Text style={styles.pollOptionHint}>Tap options in rank order (1, 2, 3...).</Text>
                    )}
                    {(state.pollOptionsByPollID[entry.poll.pollID] ?? [])
                      .filter((option) => option.isActive)
                      .sort((left, right) => left.ord - right.ord)
                      .map((option) => (
                        <TouchableOpacity
                          key={option.optionID}
                          style={[
                            styles.pollOptionRow,
                            getInlineSelected(entry.poll.pollID, option.optionID) && styles.pollOptionRowSelected,
                          ]}
                          disabled={entry.poll.status !== "open" || inlineVotingPollIDs[entry.poll.pollID]}
                          onPress={() => void handleInlineOptionPress(entry.poll.pollID, entry.poll.type, option.optionID)}
                        >
                          <VoteIndicator
                            pollType={entry.poll.type}
                            selected={getInlineSelected(entry.poll.pollID, option.optionID)}
                            rank={getInlineRank(entry.poll.pollID, option.optionID)}
                          />
                          <Text style={styles.pollOptionLabel}>{option.label}</Text>
                        </TouchableOpacity>
                      ))}
                  </View>
                )}
                {entry.poll.type === "free_text" && (
                  <View style={styles.pollOptionsList}>
                    <TextInput
                      style={styles.inlineFreeTextInput}
                      placeholder="Type your response..."
                      placeholderTextColor={appColors.textSubtle}
                      multiline
                      blurOnSubmit
                      value={state.freeTextResponseByPollID[entry.poll.pollID] ?? ""}
                      onChangeText={(value) => {
                        state.setFreeTextResponseDraft(entry.poll.pollID, value);
                        scheduleInlineFreeTextAutosave(entry.poll.pollID);
                      }}
                      onBlur={() => void state.autosaveFreeTextResponse(entry.poll.pollID, true)}
                      onSubmitEditing={() => void state.autosaveFreeTextResponse(entry.poll.pollID, true)}
                    />
                    {state.savingFreeTextByPollID[entry.poll.pollID] && (
                      <Text style={styles.pollOptionHint}>Saving...</Text>
                    )}
                  </View>
                )}
                <View style={styles.pollFooterRow}>
                  <Text style={styles.pollMeta}>{getPollStatusChip(entry.poll.expiresAt, entry.poll.status)}</Text>
                  <Text style={styles.pollMeta}>
                    {(entry.poll.type === "free_text" ? entry.poll.responseCount : entry.poll.totalVotes)} votes
                  </Text>
                </View>
                {showExactTimestamps && (
                  <Text style={[styles.exactTimestampText, styles.exactTimestampLeft]}>
                    {formatExactTimestamp(entry.createdAt)}
                  </Text>
                )}
              </TouchableOpacity>
            </View>
          );
        })}

      </KeyboardAwareScrollView>

      <View style={styles.composerRow}>
        <TouchableOpacity style={styles.pollButton} onPress={() => router.push(`/chat/${chatID}/poll/create`)}>
          <Text style={styles.pollButtonText}>Poll</Text>
        </TouchableOpacity>
        <TextInput
          style={styles.composerInput}
          value={state.messageDraft}
          onChangeText={state.setMessageDraft}
          placeholder="Type a message"
          placeholderTextColor={appColors.textSubtle}
          multiline
          blurOnSubmit={false}
          onKeyPress={(event) => {
            const nativeEvent = event.nativeEvent as { key?: string; shiftKey?: boolean };
            if (nativeEvent.key !== "Enter") {
              return;
            }

            if (nativeEvent.shiftKey) {
              return;
            }

            // Web chat UX: Enter sends; Shift+Enter keeps newline behavior.
            if (typeof (event as any).preventDefault === "function") {
              (event as any).preventDefault();
            }
            void state.createOrEditMessage();
          }}
        />
        <TouchableOpacity style={styles.sendButton} disabled={state.busy} onPress={() => void state.createOrEditMessage()}>
          <Text style={styles.sendButtonText}>➤</Text>
        </TouchableOpacity>
      </View>
    </KeyboardAvoidingView>
  );
}

type VoteIndicatorProps = {
  pollType: PollType;
  selected: boolean;
  rank: number;
};

function VoteIndicator(props: VoteIndicatorProps) {
  const { pollType, selected, rank } = props;

  if (pollType === "single_choice") {
    return (
      <View style={[styles.singleChoiceRing, selected && styles.singleChoiceRingSelected]}>
        {selected && <View style={styles.singleChoiceDot} />}
      </View>
    );
  }

  if (pollType === "multiple_choice") {
    return (
      <View style={[styles.multiChoiceBox, selected && styles.multiChoiceBoxSelected]}>
        {selected && <Text style={styles.multiChoiceCheck}>✓</Text>}
      </View>
    );
  }

  return (
    <View style={[styles.rankChoiceBadge, selected && styles.rankChoiceBadgeSelected]}>
      <Text style={[styles.rankChoiceText, selected && styles.rankChoiceTextSelected]}>
        {selected ? String(rank) : "↕"}
      </Text>
    </View>
  );
}

const styles = StyleSheet.create({
  headerActions: {
    flexDirection: "row",
    gap: 8,
  },
  thread: {
    flex: 1,
  },
  threadContent: {
    paddingHorizontal: 12,
    paddingVertical: 14,
    gap: 0,
  },
  emptyHint: {
    color: appColors.textMuted,
    fontSize: 13,
    marginBottom: 4,
  },
  loadingOlderRow: {
    alignSelf: "center",
    flexDirection: "row",
    alignItems: "center",
    gap: 8,
    marginBottom: 6,
  },
  loadingOlderText: {
    color: appColors.textMuted,
    fontSize: 12,
  },
  timeSeparatorRow: {
    alignItems: "center",
    marginTop: 14,
    marginBottom: 10,
  },
  timeSeparatorText: {
    color: appColors.textMuted,
    fontSize: 12,
    backgroundColor: appColors.surfaceRaised,
    borderColor: appColors.borderSoft,
    borderWidth: 1,
    overflow: "hidden",
    borderRadius: 10,
    paddingHorizontal: 10,
    paddingVertical: 4,
  },
  timelineEntry: {
    marginBottom: 2,
  },
  timelineGapTight: {
    marginTop: 2,
  },
  timelineGapMedium: {
    marginTop: 10,
  },
  timelineGapLarge: {
    marginTop: 18,
  },
  messageRow: {
    maxWidth: "86%",
  },
  messageRowLeft: {
    alignSelf: "flex-start",
  },
  messageRowRight: {
    alignSelf: "flex-end",
  },
  messageMeta: {
    color: appColors.textMuted,
    fontSize: 12,
    marginBottom: 4,
  },
  bubble: {
    borderRadius: 14,
    paddingHorizontal: 12,
    paddingVertical: 10,
    borderWidth: 1,
  },
  bubbleOwn: {
    backgroundColor: appColors.accent,
    borderColor: appColors.accent,
  },
  bubbleOther: {
    backgroundColor: appColors.surface,
    borderColor: appColors.borderSoft,
  },
  bubbleText: {
    color: appColors.text,
    fontSize: 16,
    lineHeight: 21,
  },
  exactTimestampText: {
    color: appColors.textMuted,
    fontSize: 11,
    marginTop: 4,
    opacity: 0.85,
  },
  exactTimestampLeft: {
    textAlign: "left",
  },
  exactTimestampRight: {
    textAlign: "right",
  },
  pollCard: {
    backgroundColor: appColors.surface,
    borderRadius: 14,
    borderWidth: 1,
    borderColor: appColors.border,
    padding: 12,
  },
  pollTag: {
    color: appColors.accent,
    fontSize: 11,
    fontWeight: "700",
    textTransform: "uppercase",
    letterSpacing: 0.6,
    marginBottom: 2,
  },
  pollQuestion: {
    color: appColors.text,
    fontWeight: "700",
    fontSize: 17,
  },
  pollOptionsList: {
    marginTop: 8,
    gap: 4,
  },
  inlineFreeTextInput: {
    minHeight: 36,
    maxHeight: 120,
    borderWidth: 1,
    borderColor: appColors.borderSoft,
    borderRadius: 10,
    color: appColors.text,
    paddingHorizontal: 10,
    paddingVertical: 8,
    backgroundColor: appColors.surfaceRaised,
    fontSize: 14,
    lineHeight: 19,
    textAlignVertical: "top",
  },
  pollOptionHint: {
    color: appColors.textMuted,
    fontSize: 12,
    marginBottom: 2,
  },
  pollOptionRow: {
    flexDirection: "row",
    alignItems: "center",
    gap: 6,
    borderRadius: 10,
    borderWidth: 1,
    borderColor: appColors.borderSoft,
    paddingHorizontal: 8,
    paddingVertical: 6,
    backgroundColor: appColors.surfaceRaised,
  },
  pollOptionRowSelected: {
    borderColor: appColors.accent,
    backgroundColor: appColors.surface,
  },
  pollOptionLabel: {
    color: appColors.textSubtle,
    fontSize: 14,
    lineHeight: 19,
    flexShrink: 1,
  },
  singleChoiceRing: {
    width: 18,
    height: 18,
    borderRadius: 9,
    borderWidth: 2,
    borderColor: appColors.textMuted,
    alignItems: "center",
    justifyContent: "center",
  },
  singleChoiceRingSelected: {
    borderColor: appColors.accent,
  },
  singleChoiceDot: {
    width: 8,
    height: 8,
    borderRadius: 4,
    backgroundColor: appColors.accent,
  },
  multiChoiceBox: {
    width: 18,
    height: 18,
    borderRadius: 5,
    borderWidth: 2,
    borderColor: appColors.textMuted,
    alignItems: "center",
    justifyContent: "center",
  },
  multiChoiceBoxSelected: {
    borderColor: appColors.accent,
    backgroundColor: appColors.accent,
  },
  multiChoiceCheck: {
    color: appColors.background,
    fontSize: 11,
    fontWeight: "800",
    lineHeight: 12,
  },
  rankChoiceBadge: {
    minWidth: 22,
    height: 18,
    borderRadius: 9,
    borderWidth: 1,
    borderColor: appColors.textMuted,
    alignItems: "center",
    justifyContent: "center",
    paddingHorizontal: 5,
  },
  rankChoiceBadgeSelected: {
    borderColor: appColors.accent,
    backgroundColor: appColors.accent,
  },
  rankChoiceText: {
    color: appColors.textMuted,
    fontSize: 11,
    fontWeight: "700",
    lineHeight: 12,
  },
  rankChoiceTextSelected: {
    color: appColors.background,
  },
  pollFooterRow: {
    marginTop: 8,
    flexDirection: "row",
    justifyContent: "space-between",
    alignItems: "center",
    gap: 12,
  },
  pollMeta: {
    color: appColors.textMuted,
    fontSize: 13,
  },
  composerRow: {
    paddingHorizontal: 12,
    paddingBottom: 12,
    paddingTop: 8,
    borderTopWidth: 1,
    borderTopColor: appColors.borderSoft,
    backgroundColor: appColors.background,
    flexDirection: "row",
    alignItems: "center",
    gap: 8,
  },
  composerInput: {
    flex: 1,
    backgroundColor: appColors.surface,
    borderRadius: 20,
    borderWidth: 1,
    borderColor: appColors.border,
    color: appColors.text,
    paddingHorizontal: 14,
    paddingVertical: 10,
    fontSize: 16,
  },
  pollButton: {
    backgroundColor: appColors.surfaceRaised,
    borderWidth: 1,
    borderColor: appColors.border,
    borderRadius: 16,
    paddingVertical: 10,
    paddingHorizontal: 12,
  },
  pollButtonText: {
    color: appColors.textSubtle,
    fontWeight: "700",
    fontSize: 13,
  },
  sendButton: {
    width: 38,
    height: 38,
    borderRadius: 19,
    alignItems: "center",
    justifyContent: "center",
    backgroundColor: appColors.accent,
  },
  sendButtonText: {
    color: appColors.text,
    fontWeight: "700",
    fontSize: 16,
    marginLeft: 1,
  },
});
