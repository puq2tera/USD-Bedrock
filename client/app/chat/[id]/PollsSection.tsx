import { Text, TouchableOpacity, View } from "react-native";
import { PollSummary, getIdentityLabel } from "../../../lib/api";
import { commonStyles } from "../../../lib/styles";
import { chatDetailStyles as styles } from "./chatDetailStyles";

type PollsSectionProps = {
  polls: PollSummary[];
  onCreatePoll: () => void;
  onOpenPoll: (pollID: string) => void;
};

export function PollsSection({ polls, onCreatePoll, onOpenPoll }: PollsSectionProps) {
  return (
    <View style={commonStyles.sectionCard}>
      <View style={commonStyles.sectionHeaderRow}>
        <Text style={commonStyles.sectionTitle}>Polls</Text>
        <TouchableOpacity style={commonStyles.miniPrimaryButton} onPress={onCreatePoll}>
          <Text style={commonStyles.miniPrimaryButtonText}>Create</Text>
        </TouchableOpacity>
      </View>

      {polls.length < 1 && <Text style={commonStyles.metaText}>No polls in this chat yet.</Text>}
      {polls.map((poll) => (
        <TouchableOpacity key={poll.pollID} style={[commonStyles.outlinedRow, styles.pollRow]} onPress={() => onOpenPoll(poll.pollID)}>
          <Text style={[commonStyles.emphasizedRowLabel, styles.pollQuestion]}>{poll.question}</Text>
          <Text style={commonStyles.metaText}>{poll.type} • {poll.status} • by {getIdentityLabel(poll.creatorUserID)}</Text>
        </TouchableOpacity>
      ))}
    </View>
  );
}
