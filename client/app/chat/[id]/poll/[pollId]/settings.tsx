import { useCallback, useEffect, useState } from "react";
import { RefreshControl, StyleSheet, Switch, Text, TextInput, TouchableOpacity, View } from "react-native";
import { Redirect, useFocusEffect, useLocalSearchParams } from "expo-router";
import { KeyboardAwareScrollView } from "../../../../../components/KeyboardAwareScrollView";
import TypescriptUtils from "../../../../../lib/TypescriptUtils";
import { useAuth } from "../../../../../lib/auth";
import { appColors, commonStyles } from "../../../../../lib/styles";
import { usePollDetailState } from "./usePollDetailState";

export default function PollSettingsScreen() {
  const { isAuthenticated, user } = useAuth();
  const { id, pollId } = useLocalSearchParams<{ id: string; pollId: string }>();
  const chatID = TypescriptUtils.parseString(id) ?? "";
  const resolvedPollID = TypescriptUtils.parseString(pollId) ?? "";

  const state = usePollDetailState(chatID, resolvedPollID, user?.userID ?? "");
  const [showSavedBanner, setShowSavedBanner] = useState(false);

  useEffect(() => {
    if (state.lastPollEditSavedAt < 1) {
      return;
    }

    setShowSavedBanner(true);
    const timeout = setTimeout(() => setShowSavedBanner(false), 2500);
    return () => clearTimeout(timeout);
  }, [state.lastPollEditSavedAt]);

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
    <KeyboardAwareScrollView
      style={commonStyles.screen}
      contentContainerStyle={commonStyles.screenContent}
      refreshControl={<RefreshControl refreshing={state.refreshing} onRefresh={() => {
        state.setRefreshing(true);
        void state.loadPoll();
      }} />}
    >
      <View style={commonStyles.sectionCard}>
        <Text style={commonStyles.sectionTitle}>Poll Settings</Text>
        <Text style={commonStyles.metaText}>Review your edits, then save changes.</Text>
        {showSavedBanner && (
          <View style={[commonStyles.feedbackBanner, commonStyles.successBanner]}>
            <Text style={commonStyles.successBannerText}>Poll settings saved.</Text>
          </View>
        )}

        <Text style={commonStyles.sectionLabel}>Question</Text>
        <TextInput
          style={commonStyles.input}
          value={state.editQuestion}
          onChangeText={state.setEditQuestion}
        />

        <View style={commonStyles.formSwitchRow}>
          <Text style={commonStyles.formSwitchLabel}>Allow vote changes</Text>
          <Switch value={state.editAllowChangeVote} onValueChange={state.setEditAllowChangeVote} />
        </View>

        <View style={commonStyles.formSwitchRow}>
          <Text style={commonStyles.formSwitchLabel}>Anonymous</Text>
          <Switch value={state.editIsAnonymous} onValueChange={state.setEditIsAnonymous} />
        </View>

        <Text style={commonStyles.sectionLabel}>Expires at</Text>
        <TextInput
          style={commonStyles.input}
          value={state.editExpiresAt}
          onChangeText={state.setEditExpiresAt}
          placeholder="2026-05-01T18:00"
          placeholderTextColor={appColors.textSubtle}
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
          </>
        )}

        <TouchableOpacity style={[commonStyles.primaryButton, styles.saveButton]} onPress={() => void state.submitPollSettings()}>
          <Text style={commonStyles.primaryButtonText}>Save Poll Settings</Text>
        </TouchableOpacity>
      </View>
    </KeyboardAwareScrollView>
  );
}

const styles = StyleSheet.create({
  optionInput: {
    marginBottom: 8,
  },
  saveButton: {
    marginTop: 16,
  },
});
