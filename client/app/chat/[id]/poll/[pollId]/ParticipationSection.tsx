import { useEffect, useRef } from "react";
import { Text, TextInput, TouchableOpacity, View } from "react-native";
import { PollDetail, PollOption } from "../../../../../lib/api";
import { appColors, commonStyles } from "../../../../../lib/styles";
import { pollDetailStyles as styles } from "./pollDetailStyles";

type ParticipationSectionProps = {
  poll: PollDetail;
  selectedOptionIDs: string[];
  textResponse: string;
  busy: boolean;
  savingTextResponse: boolean;
  onToggleOption: (option: PollOption) => Promise<void>;
  onTextResponseChange: (value: string) => void;
  onAutosaveTextResponse: (force?: boolean) => Promise<void>;
  onRemoveParticipation: () => Promise<void>;
};

export function ParticipationSection(props: ParticipationSectionProps) {
  const {
    poll,
    selectedOptionIDs,
    textResponse,
    busy,
    savingTextResponse,
    onToggleOption,
    onTextResponseChange,
    onAutosaveTextResponse,
    onRemoveParticipation,
  } = props;

  const debounceTimer = useRef<ReturnType<typeof setTimeout> | null>(null);

  useEffect(() => {
    if (poll.type !== "free_text") {
      return;
    }

    if (debounceTimer.current !== null) {
      clearTimeout(debounceTimer.current);
    }

    debounceTimer.current = setTimeout(() => {
      void onAutosaveTextResponse(false);
    }, 1200);

    return () => {
      if (debounceTimer.current !== null) {
        clearTimeout(debounceTimer.current);
      }
    };
  }, [onAutosaveTextResponse, poll.type, textResponse]);

  const getRank = (optionID: string): number => selectedOptionIDs.indexOf(optionID) + 1;
  const getIsSelected = (optionID: string): boolean => selectedOptionIDs.includes(optionID);

  return (
    <View style={commonStyles.sectionCard}>
      <Text style={commonStyles.sectionTitle}>Participation</Text>
      {poll.type === "free_text" ? (
        <>
          <Text style={[commonStyles.metaText, styles.freeTextIntro]}>
            Write your response. It auto-saves when you pause typing.
          </Text>
          <TextInput
            style={[commonStyles.input, styles.multilineInput]}
            value={textResponse}
            onChangeText={onTextResponseChange}
            placeholder="Your response"
            placeholderTextColor={appColors.textSubtle}
            multiline
            blurOnSubmit
            onBlur={() => void onAutosaveTextResponse(true)}
            onSubmitEditing={() => void onAutosaveTextResponse(true)}
          />
          {savingTextResponse && <Text style={[commonStyles.metaText, styles.selectionHint]}>Saving...</Text>}
        </>
      ) : (
        <>
          {poll.type === "ranked_choice" && (
            <Text style={[commonStyles.metaText, styles.selectionHint]}>Tap options in rank order (1, 2, 3...).</Text>
          )}
          {poll.options.map((option) => {
            const selected = getIsSelected(option.optionID);
            const rank = getRank(option.optionID);
            return (
              <TouchableOpacity
                key={option.optionID}
                style={[commonStyles.outlinedRow, styles.optionRow, selected && styles.optionRowSelected]}
                onPress={() => void onToggleOption(option)}
              >
                <View style={styles.optionLeading}>
                  {poll.type === "single_choice" && (
                    <View style={[styles.singleChoiceRing, selected && styles.singleChoiceRingSelected]}>
                      {selected && <View style={styles.singleChoiceDot} />}
                    </View>
                  )}
                  {poll.type === "multiple_choice" && (
                    <View style={[styles.multiChoiceBox, selected && styles.multiChoiceBoxSelected]}>
                      {selected && <Text style={styles.multiChoiceCheck}>✓</Text>}
                    </View>
                  )}
                  {poll.type === "ranked_choice" && (
                    <View style={[styles.rankChoiceBadge, selected && styles.rankChoiceBadgeSelected]}>
                      <Text style={[styles.rankChoiceText, selected && styles.rankChoiceTextSelected]}>
                        {selected ? String(rank) : "↕"}
                      </Text>
                    </View>
                  )}
                </View>
                <Text style={styles.optionText}>{option.label}</Text>
                <Text style={commonStyles.metaText}>{option.voteCount} votes</Text>
              </TouchableOpacity>
            );
          })}
        </>
      )}
    </View>
  );
}
