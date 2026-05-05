import { Text, View } from "react-native";
import { getIdentityLabel, PollParticipation } from "../../../../../lib/api";
import { commonStyles } from "../../../../../lib/styles";
import { pollDetailStyles as styles } from "./pollDetailStyles";

type ParticipationSummarySectionProps = {
  participation: PollParticipation | null;
};

export function ParticipationSummarySection({ participation }: ParticipationSummarySectionProps) {
  return (
    <View style={commonStyles.sectionCard}>
      <Text style={commonStyles.sectionTitle}>Participation Summary</Text>
      {!participation && (
        <View style={[commonStyles.feedbackBanner, commonStyles.infoBanner]}>
          <Text style={commonStyles.infoBannerText}>No participation data yet. Ask members to vote to populate this summary.</Text>
        </View>
      )}
      {participation && (
        <>
          <View style={styles.countRow}>
            <View style={styles.countChip}>
              <Text style={styles.countValue}>{participation.eligibleCount}</Text>
              <Text style={styles.countLabel}>Eligible</Text>
            </View>
            <View style={styles.countChip}>
              <Text style={styles.countValue}>{participation.votedCount}</Text>
              <Text style={styles.countLabel}>Voted</Text>
            </View>
            <View style={styles.countChip}>
              <Text style={styles.countValue}>{participation.notVotedCount}</Text>
              <Text style={styles.countLabel}>Not voted</Text>
            </View>
          </View>
          {!participation.isAnonymous ? (
            <>
              <Text style={commonStyles.listTitle}>Voted users</Text>
              {participation.votedUserIDs.length < 1 && <Text style={commonStyles.metaText}>No votes yet.</Text>}
              {participation.votedUserIDs.map((userID) => <Text key={`voted-${userID}`} style={commonStyles.metaText}>• {getIdentityLabel(userID)}</Text>)}
              <Text style={commonStyles.listTitle}>Not voted users</Text>
              {participation.notVotedUserIDs.length < 1 && <Text style={commonStyles.metaText}>Everyone has voted.</Text>}
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
