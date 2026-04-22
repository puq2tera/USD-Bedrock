import { useCallback } from "react";
import { Alert, ScrollView, StyleSheet, Switch, Text, TextInput, TouchableOpacity, View } from "react-native";
import { Redirect, useFocusEffect, useLocalSearchParams } from "expo-router";
import TypescriptUtils from "../../../../../lib/TypescriptUtils";
import { useAuth } from "../../../../../lib/auth";
import { commonStyles } from "../../../../../lib/styles";
import { usePollDetailState } from "./usePollDetailState";

export default function PollSettingsScreen() {
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

  if (!state.poll || !state.isCreator) {
    return (
      <View style={commonStyles.centeredScreen}>
        <Text style={commonStyles.errorText}>Only the poll creator can edit settings.</Text>
      </View>
    );
  }

  return (
    <ScrollView style={commonStyles.screen} contentContainerStyle={commonStyles.screenContent}>
      <View style={commonStyles.sectionCard}>
        <Text style={commonStyles.sectionTitle}>Poll Settings</Text>
        <Text style={commonStyles.metaText}>Changes save instantly.</Text>

        <Text style={commonStyles.sectionLabel}>Question</Text>
        <TextInput
          style={commonStyles.input}
          value={state.editQuestion}
          onChangeText={state.setEditQuestion}
          onBlur={() => void state.submitPollEdit()}
        />

        <View style={commonStyles.formSwitchRow}>
          <Text style={commonStyles.formSwitchLabel}>Allow vote changes</Text>
          <Switch
            value={state.editAllowChangeVote}
            onValueChange={(value) => {
              state.setEditAllowChangeVote(value);
              void state.submitPollEdit({ allowChangeVote: value });
            }}
          />
        </View>

        <View style={commonStyles.formSwitchRow}>
          <Text style={commonStyles.formSwitchLabel}>Anonymous</Text>
          <Switch
            value={state.editIsAnonymous}
            onValueChange={(value) => {
              state.setEditIsAnonymous(value);
              void state.submitPollEdit({ isAnonymous: value });
            }}
          />
        </View>

        <Text style={commonStyles.sectionLabel}>Expires at</Text>
        <TextInput
          style={commonStyles.input}
          value={state.editExpiresAt}
          onChangeText={state.setEditExpiresAt}
          onBlur={() => void state.submitPollEdit()}
          placeholder="2026-05-01T18:00"
        />

        {state.poll.type !== "free_text" && (
          <>
            <Text style={commonStyles.sectionLabel}>Options</Text>
            {state.editOptions.map((label, index) => (
              <TextInput
                key={`option-${index}`}
                style={[commonStyles.input, styles.optionInput]}
                value={label}
                onChangeText={(nextLabel) => {
                  const next = [...state.editOptions];
                  next[index] = nextLabel;
                  state.setEditOptions(next);
                }}
              />
            ))}
            <TouchableOpacity style={commonStyles.ghostButton} onPress={() => void state.confirmReplaceOptions()}>
              <Text style={commonStyles.ghostButtonText}>Save option changes</Text>
            </TouchableOpacity>
          </>
        )}
      </View>

      <View style={styles.bottomActions}>
        {state.poll.status === "open" ? (
          <TouchableOpacity style={commonStyles.miniDangerButton} onPress={() => state.transitionStatus("closed")}>
            <Text style={commonStyles.miniDangerButtonText}>Close poll</Text>
          </TouchableOpacity>
        ) : (
          <TouchableOpacity style={commonStyles.miniPrimaryButton} onPress={() => state.transitionStatus("open")}>
            <Text style={commonStyles.miniPrimaryButtonText}>Reopen poll</Text>
          </TouchableOpacity>
        )}

        <TouchableOpacity
          style={commonStyles.ghostButton}
          onPress={() =>
            Alert.alert("Reset my vote", "Remove your current participation?", [
              { text: "Cancel", style: "cancel" },
              { text: "Reset", style: "destructive", onPress: () => void state.removeParticipation() },
            ])
          }
        >
          <Text style={commonStyles.ghostButtonText}>Reset my vote</Text>
        </TouchableOpacity>

        <TouchableOpacity style={commonStyles.miniDangerButton} onPress={() => state.removePoll()}>
          <Text style={commonStyles.miniDangerButtonText}>Delete poll</Text>
        </TouchableOpacity>
      </View>
    </ScrollView>
  );
}

const styles = StyleSheet.create({
  optionInput: {
    marginBottom: 8,
  },
  bottomActions: {
    marginTop: 12,
    flexDirection: "row",
    justifyContent: "space-between",
    alignItems: "center",
  },
});
