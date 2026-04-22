import { Text, View } from "react-native";
import { getIdentityLabel, PollParticipation } from "../../../../../lib/api";
import { commonStyles } from "../../../../../lib/styles";

type ParticipationSummarySectionProps = {
  participation: PollParticipation | null;
};

export function ParticipationSummarySection({ participation }: ParticipationSummarySectionProps) {
  return (
    <View style={commonStyles.sectionCard}>
      <Text style={commonStyles.sectionTitle}>Participation Summary</Text>
      {participation && (
        <>
          <Text style={commonStyles.metaText}>Eligible: {participation.eligibleCount}</Text>
          <Text style={commonStyles.metaText}>Voted: {participation.votedCount}</Text>
          <Text style={commonStyles.metaText}>Not voted: {participation.notVotedCount}</Text>
          {!participation.isAnonymous ? (
            <>
              <Text style={commonStyles.listTitle}>Voted users</Text>
              {participation.votedUserIDs.map((userID) => <Text key={`voted-${userID}`} style={commonStyles.metaText}>• {getIdentityLabel(userID)}</Text>)}
              <Text style={commonStyles.listTitle}>Not voted users</Text>
              {participation.notVotedUserIDs.map((userID) => <Text key={`not-voted-${userID}`} style={commonStyles.metaText}>• {getIdentityLabel(userID)}</Text>)}
            </>
          ) : (
            <Text style={commonStyles.metaText}>Anonymous poll: user identities are hidden.</Text>
          )}
        </>
      )}
    </View>
  );
}
