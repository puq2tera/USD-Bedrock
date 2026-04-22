import { Text, TextInput, TouchableOpacity, View } from "react-native";
import { PollDetail, PollOption } from "../../../../../lib/api";
import { commonStyles } from "../../../../../lib/styles";
import { pollDetailStyles as styles } from "./pollDetailStyles";

type ParticipationSectionProps = {
  poll: PollDetail;
  selectedOptionIDs: string[];
  textResponse: string;
  busy: boolean;
  onToggleOption: (option: PollOption) => Promise<void>;
  onTextResponseChange: (value: string) => void;
  onSubmit: () => Promise<void>;
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
          <Text style={commonStyles.metaText}>Tap an option to vote. Tap again to remove your vote.</Text>
          {poll.options.map((option) => {
            const selected = selectedOptionIDs.includes(option.optionID);
            return (
              <TouchableOpacity
                key={option.optionID}
                style={[commonStyles.outlinedRow, styles.optionRow, selected && styles.optionRowSelected]}
                onPress={() => void onToggleOption(option)}
              >
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
