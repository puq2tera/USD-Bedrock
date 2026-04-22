import { Text, TextInput, TouchableOpacity, View } from "react-native";
import { PollDetail, PollOption } from "../../../../../lib/api";
import { commonStyles } from "../../../../../lib/styles";
import { pollDetailStyles as styles } from "./pollDetailStyles";

type ParticipationSectionProps = {
  poll: PollDetail;
  selectedOptionIDs: string[];
  textResponse: string;
  busy: boolean;
  onToggleOption: (option: PollOption) => void;
  onTextResponseChange: (value: string) => void;
  onSubmit: () => Promise<void>;
  onRemoveParticipation: () => Promise<void>;
};

export function ParticipationSection(props: ParticipationSectionProps) {
  const {
    poll,
    selectedOptionIDs,
    textResponse,
    busy,
    onToggleOption,
    onTextResponseChange,
    onSubmit,
    onRemoveParticipation,
  } = props;

  return (
    <View style={commonStyles.sectionCard}>
      <Text style={commonStyles.sectionTitle}>Participation</Text>
      {poll.type === "free_text" ? (
        <>
          <TextInput
            style={[commonStyles.input, styles.multilineInput]}
            value={textResponse}
            onChangeText={onTextResponseChange}
            placeholder="Your response"
            multiline
          />
          <TouchableOpacity style={commonStyles.primaryButton} disabled={busy} onPress={() => void onSubmit()}>
            <Text style={commonStyles.primaryButtonText}>{busy ? "Saving..." : "Submit response"}</Text>
          </TouchableOpacity>
        </>
      ) : (
        <>
          {poll.options.map((option) => {
            const selected = selectedOptionIDs.includes(option.optionID);
            return (
              <TouchableOpacity
                key={option.optionID}
                style={[commonStyles.outlinedRow, styles.optionRow, selected && styles.optionRowSelected]}
                onPress={() => onToggleOption(option)}
              >
                <Text style={styles.optionText}>{option.label}</Text>
                <Text style={commonStyles.metaText}>{option.voteCount} votes</Text>
              </TouchableOpacity>
            );
          })}

          <TouchableOpacity style={commonStyles.primaryButton} disabled={busy} onPress={() => void onSubmit()}>
            <Text style={commonStyles.primaryButtonText}>{busy ? "Submitting..." : "Submit selection"}</Text>
          </TouchableOpacity>
        </>
      )}

      <TouchableOpacity style={commonStyles.ghostButton} onPress={() => void onRemoveParticipation()}>
        <Text style={commonStyles.ghostButtonText}>Remove my participation</Text>
      </TouchableOpacity>
    </View>
  );
}
