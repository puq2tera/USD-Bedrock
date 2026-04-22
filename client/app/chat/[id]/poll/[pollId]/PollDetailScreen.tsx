import { useCallback } from "react";
import { ActivityIndicator, RefreshControl, ScrollView, Text, TouchableOpacity, View } from "react-native";
import { Redirect, useFocusEffect, useLocalSearchParams } from "expo-router";
import TypescriptUtils from "../../../../../lib/TypescriptUtils";
import { getIdentityLabel } from "../../../../../lib/api";
import { useAuth } from "../../../../../lib/auth";
import { appColors, commonStyles } from "../../../../../lib/styles";
import { CreatorControlsSection } from "./CreatorControlsSection";
import { ParticipationSection } from "./ParticipationSection";
import { ParticipationSummarySection } from "./ParticipationSummarySection";
import { pollDetailStyles as styles } from "./pollDetailStyles";
import { usePollDetailState } from "./usePollDetailState";

export function PollDetailScreen() {
  const { isAuthenticated, user } = useAuth();
  const { id, pollId } = useLocalSearchParams<{ id: string; pollId: string }>();
  const chatID = TypescriptUtils.parseString(id) ?? "";
  const resolvedPollID = TypescriptUtils.parseString(pollId) ?? "";

  const state = usePollDetailState(chatID, resolvedPollID, user?.userID ?? "");

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
    <ScrollView
      style={commonStyles.screen}
      contentContainerStyle={commonStyles.screenContent}
      refreshControl={<RefreshControl refreshing={state.refreshing} onRefresh={() => {
        state.setRefreshing(true);
        void state.loadPoll();
      }} />}
    >
      <View style={commonStyles.sectionCard}>
        <Text style={[commonStyles.pageTitle, styles.question]}>{state.poll.question}</Text>
        <Text style={[commonStyles.metaText, styles.metaWithTop]}>{state.poll.type} • {state.poll.status} • by {getIdentityLabel(state.poll.creatorUserID)}</Text>
        <Text style={[commonStyles.metaText, styles.metaWithTop]}>Votes: {state.poll.totalVotes} • Voters: {state.poll.totalVoters}</Text>
        {state.poll.type === "free_text" && <Text style={[commonStyles.metaText, styles.metaWithTop]}>Responses: {state.poll.responseCount}</Text>}
      </View>

      <ParticipationSection
        poll={state.poll}
        selectedOptionIDs={state.selectedOptionIDs}
        textResponse={state.textResponse}
        busy={state.busy}
        onToggleOption={state.toggleOptionSelection}
        onTextResponseChange={state.setTextResponse}
        onSubmit={state.submitVoteSelection}
        onRemoveParticipation={state.removeParticipation}
      />

      <ParticipationSummarySection participation={state.participation} />

      {state.isCreator && (
        <CreatorControlsSection
          poll={state.poll}
          editing={state.editing}
          editQuestion={state.editQuestion}
          editAllowChangeVote={state.editAllowChangeVote}
          editIsAnonymous={state.editIsAnonymous}
          editExpiresAt={state.editExpiresAt}
          editOptions={state.editOptions}
          setEditing={state.setEditing}
          setEditQuestion={state.setEditQuestion}
          setEditAllowChangeVote={state.setEditAllowChangeVote}
          setEditIsAnonymous={state.setEditIsAnonymous}
          setEditExpiresAt={state.setEditExpiresAt}
          setEditOptions={state.setEditOptions}
          onSubmitEdit={() => state.submitPollEdit()}
          onConfirmReplaceOptions={state.confirmReplaceOptions}
          onTransitionStatus={state.transitionStatus}
          onDeletePoll={state.removePoll}
        />
      )}
    </ScrollView>
  );
}
