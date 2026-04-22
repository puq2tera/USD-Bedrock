import { useCallback, useLayoutEffect } from "react";
import {
  ActivityIndicator,
  KeyboardAvoidingView,
  Platform,
  RefreshControl,
  ScrollView,
  StyleSheet,
  Text,
  TextInput,
  TouchableOpacity,
  View,
} from "react-native";
import { Redirect, useFocusEffect, useLocalSearchParams, useNavigation, useRouter } from "expo-router";
import TypescriptUtils from "../../../lib/TypescriptUtils";
import { getIdentityLabel } from "../../../lib/api";
import { useAuth } from "../../../lib/auth";
import { appColors, commonStyles } from "../../../lib/styles";
import { useChatDetailState } from "./useChatDetailState";

export function ChatDetailScreen() {
  const { isAuthenticated, user } = useAuth();
  const router = useRouter();
  const navigation = useNavigation();
  const { id } = useLocalSearchParams<{ id: string }>();
  const chatID = TypescriptUtils.parseString(id) ?? "";

  const state = useChatDetailState(chatID);

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

  return (
    <KeyboardAvoidingView style={commonStyles.screen} behavior={Platform.OS === "ios" ? "padding" : undefined}>
      <ScrollView
        style={styles.thread}
        contentContainerStyle={styles.threadContent}
        refreshControl={<RefreshControl refreshing={state.refreshing} onRefresh={() => {
          state.setRefreshing(true);
          void state.loadAll();
        }} />}
      >
        {state.messages.map((message) => {
          const isOwnMessage = message.userID === user?.userID;
          return (
            <View key={message.messageID} style={[styles.messageRow, isOwnMessage ? styles.messageRowRight : styles.messageRowLeft]}>
              <Text style={styles.messageMeta}>{isOwnMessage ? "You" : getIdentityLabel(message.userID)}</Text>
              <View style={[styles.bubble, isOwnMessage ? styles.bubbleOwn : styles.bubbleOther]}>
                <Text style={styles.bubbleText}>{message.body}</Text>
              </View>
            </View>
          );
        })}

        {state.polls.map((poll) => (
          <TouchableOpacity key={poll.pollID} style={styles.pollCard} onPress={() => router.push(`/chat/${chatID}/poll/${poll.pollID}`)}>
            <Text style={styles.pollQuestion}>{poll.question}</Text>
            <Text style={styles.pollMeta}>{poll.status === "open" ? "Active" : "Closed"} • {poll.totalVotes} votes</Text>
          </TouchableOpacity>
        ))}

        {state.nextBeforeMessageID && (
          <TouchableOpacity style={styles.loadMoreButton} disabled={state.loadingMoreMessages} onPress={() => void state.loadOlderMessages()}>
            <Text style={styles.loadMoreButtonText}>{state.loadingMoreMessages ? "Loading..." : "Load older messages"}</Text>
          </TouchableOpacity>
        )}
      </ScrollView>

      <View style={styles.composerRow}>
        <TouchableOpacity style={styles.pollButton} onPress={() => router.push(`/chat/${chatID}/poll/create`)}>
          <Text style={styles.pollButtonText}>Poll</Text>
        </TouchableOpacity>
        <TextInput
          style={styles.composerInput}
          value={state.messageDraft}
          onChangeText={state.setMessageDraft}
          placeholder="Type a message"
          placeholderTextColor={appColors.textMuted}
        />
        <TouchableOpacity style={styles.sendButton} disabled={state.busy} onPress={() => void state.createOrEditMessage()}>
          <Text style={styles.sendButtonText}>➤</Text>
        </TouchableOpacity>
      </View>
    </KeyboardAvoidingView>
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
    gap: 8,
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
  pollCard: {
    marginTop: 8,
    backgroundColor: appColors.surface,
    borderRadius: 14,
    borderWidth: 1,
    borderColor: appColors.border,
    padding: 12,
  },
  pollQuestion: {
    color: appColors.text,
    fontWeight: "700",
    fontSize: 17,
  },
  pollMeta: {
    color: appColors.textMuted,
    marginTop: 4,
    fontSize: 13,
  },
  loadMoreButton: {
    alignSelf: "center",
    marginTop: 10,
    marginBottom: 6,
    backgroundColor: appColors.surfaceRaised,
    borderWidth: 1,
    borderColor: appColors.border,
    borderRadius: 999,
    paddingHorizontal: 14,
    paddingVertical: 8,
  },
  loadMoreButtonText: {
    color: appColors.textSubtle,
    fontWeight: "600",
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
