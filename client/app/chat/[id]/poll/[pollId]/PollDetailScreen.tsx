import { useCallback, useLayoutEffect } from "react";
import { ActivityIndicator, RefreshControl, StyleSheet, Text, TouchableOpacity, View } from "react-native";
import { Redirect, useFocusEffect, useLocalSearchParams, useNavigation, useRouter } from "expo-router";
import { KeyboardAwareScrollView } from "../../../../../components/KeyboardAwareScrollView";
import TypescriptUtils from "../../../../../lib/TypescriptUtils";
import { getIdentityLabel } from "../../../../../lib/api";
import { useAuth } from "../../../../../lib/auth";
import { appColors, commonStyles } from "../../../../../lib/styles";
import { ParticipationSection } from "./ParticipationSection";
import { ParticipationSummarySection } from "./ParticipationSummarySection";
import { pollDetailStyles as styles } from "./pollDetailStyles";
import { usePollDetailState } from "./usePollDetailState";

export function PollDetailScreen() {
  const { isAuthenticated, user } = useAuth();
  const router = useRouter();
  const navigation = useNavigation();
  const { id, pollId } = useLocalSearchParams<{ id: string; pollId: string }>();
  const chatID = TypescriptUtils.parseString(id) ?? "";
  const resolvedPollID = TypescriptUtils.parseString(pollId) ?? "";

  const state = usePollDetailState(chatID, resolvedPollID, user?.userID ?? "");

  useLayoutEffect(() => {
    navigation.setOptions({
      headerRight: () => (
        <View style={localStyles.headerActions}>
          {state.isCreator && (
            <TouchableOpacity style={commonStyles.circularIconButton} onPress={() => router.push(`/chat/${chatID}/poll/${resolvedPollID}/settings`)}>
              <Text style={commonStyles.circularIconButtonText}>⚙</Text>
            </TouchableOpacity>
          )}
          <TouchableOpacity style={commonStyles.circularIconButton} onPress={() => router.push("/account")}>
            <Text style={commonStyles.circularIconButtonText}>👤</Text>
          </TouchableOpacity>
        </View>
      ),
    });
  }, [chatID, navigation, resolvedPollID, router, state.isCreator]);

  useFocusEffect(
    useCallback(() => {
      void state.loadPoll();
    }, [state.loadPoll])
  );

  if (!isAuthenticated) {
    return <Redirect href="/login" />;
  }

  if (state.loading) {
    return (
      <View style={commonStyles.centeredScreen}>
        <ActivityIndicator size="large" color={appColors.accent} />
      </View>
    );
  }

  if (!state.poll || state.error) {
    return (
      <View style={commonStyles.centeredScreen}>
        <Text style={commonStyles.errorText}>{state.error || "Poll not found"}</Text>
        <TouchableOpacity style={commonStyles.retryButton} onPress={() => void state.loadPoll()}>
          <Text style={commonStyles.retryButtonText}>Retry</Text>
        </TouchableOpacity>
      </View>
    );
  }

  return (
    <KeyboardAwareScrollView
      style={commonStyles.screen}
      contentContainerStyle={commonStyles.screenContent}
      refreshControl={<RefreshControl refreshing={state.refreshing} onRefresh={() => {
        state.setRefreshing(true);
        void state.loadPoll();
      }} />}
    >
      <View style={commonStyles.sectionCard}>
        <Text style={[commonStyles.pageTitle, styles.question]}>{state.poll.question}</Text>
        <Text style={[commonStyles.metaText, styles.metaWithTop]}>
          {state.poll.status === "open" ? "Active" : "Closed"} • by {getIdentityLabel(state.poll.creatorUserID)}
        </Text>
        <Text style={[commonStyles.metaText, styles.metaWithTop]}>
          {state.poll.totalVotes} votes • {state.poll.totalVoters} voters
        </Text>
      </View>

      <ParticipationSection
        poll={state.poll}
        selectedOptionIDs={state.selectedOptionIDs}
        textResponse={state.textResponse}
        busy={state.busy}
        savingTextResponse={state.savingTextResponse}
        onToggleOption={state.toggleOptionSelection}
        onTextResponseChange={state.setTextResponse}
        onAutosaveTextResponse={state.autosaveTextResponse}
        onRemoveParticipation={state.removeParticipation}
      />

      <ParticipationSummarySection participation={state.participation} />
    </KeyboardAwareScrollView>
  );
}

const localStyles = StyleSheet.create({
  headerActions: {
    flexDirection: "row",
    gap: 8,
  },
});
