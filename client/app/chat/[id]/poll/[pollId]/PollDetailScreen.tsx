import { useCallback, useLayoutEffect } from "react";
import { ActivityIndicator, Alert, RefreshControl, StyleSheet, Text, TouchableOpacity, View } from "react-native";
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
              <Text style={commonStyles.circularIconButtonText}>✎</Text>
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
        <View style={styles.summaryMetaRow}>
          <View style={[styles.stateChip, state.poll.status === "open" ? styles.stateChipOpen : styles.stateChipClosed]}>
            <Text style={styles.stateChipText}>{state.poll.status === "open" ? "Active" : "Closed"}</Text>
          </View>
          <Text style={commonStyles.metaText}>by {getIdentityLabel(state.poll.creatorUserID)}</Text>
        </View>
        <View style={styles.summaryStatsRow}>
          <Text style={styles.summaryStatValue}>{state.poll.totalVotes}</Text>
          <Text style={styles.summaryStatLabel}>total votes</Text>
          <Text style={styles.summaryStatDivider}>•</Text>
          <Text style={styles.summaryStatValue}>{state.poll.totalVoters}</Text>
          <Text style={styles.summaryStatLabel}>participants</Text>
        </View>
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

      {state.isCreator && (
        <View style={commonStyles.sectionCard}>
          {state.poll.status === "open" ? (
            <TouchableOpacity style={[commonStyles.primaryButton, localStyles.adminActionButton]} onPress={() => state.transitionStatus("closed")}>
              <Text style={commonStyles.primaryButtonText}>Close Poll</Text>
            </TouchableOpacity>
          ) : (
            <TouchableOpacity style={[commonStyles.primaryButton, localStyles.adminActionButton]} onPress={() => state.transitionStatus("open")}>
              <Text style={commonStyles.primaryButtonText}>Reopen Poll</Text>
            </TouchableOpacity>
          )}

          <TouchableOpacity
            style={[commonStyles.primaryButton, localStyles.adminActionButton]}
            onPress={() =>
              Alert.alert("Reset My Vote", "Remove your current participation?", [
                { text: "Cancel", style: "cancel" },
                { text: "Reset", style: "destructive", onPress: () => void state.removeParticipation() },
              ])
            }
          >
            <Text style={commonStyles.primaryButtonText}>Reset My Vote</Text>
          </TouchableOpacity>

          <TouchableOpacity
            style={[commonStyles.primaryButton, localStyles.adminActionButton]}
            onPress={() =>
              Alert.alert("Reset All Votes", "Delete all participation on this poll?", [
                { text: "Cancel", style: "cancel" },
                { text: "Reset All", style: "destructive", onPress: () => void state.removeAllParticipations() },
              ])
            }
          >
            <Text style={commonStyles.primaryButtonText}>Reset All Votes</Text>
          </TouchableOpacity>

          <TouchableOpacity style={[commonStyles.dangerBlockButton, localStyles.adminActionButton]} onPress={() => state.removePoll()}>
            <Text style={commonStyles.dangerBlockButtonText}>Delete Poll</Text>
          </TouchableOpacity>
        </View>
      )}
    </KeyboardAwareScrollView>
  );
}

const localStyles = StyleSheet.create({
  headerActions: {
    flexDirection: "row",
    gap: 8,
  },
  adminActionButton: {
    marginTop: 10,
  },
});
