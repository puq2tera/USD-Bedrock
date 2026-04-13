import { useCallback, useState } from "react";
import {
  View,
  Text,
  TouchableOpacity,
  StyleSheet,
  ActivityIndicator,
  ScrollView,
  RefreshControl,
  Alert,
} from "react-native";
import { Redirect, useLocalSearchParams } from "expo-router";
import { useFocusEffect } from "expo-router";
import { getPoll, submitVote, PollDetail, PollOption } from "../../lib/api";
import TypescriptUtils from "../../lib/TypescriptUtils";
import { useAuth } from "../../lib/auth";
import { appColors, commonStyles } from "../../lib/styles";

export default function PollDetailScreen() {
  const { isAuthenticated } = useAuth();

  if (!isAuthenticated) {
    return <Redirect href="/login" />;
  }

  const { id } = useLocalSearchParams<{ id: string }>();
  const pollID = TypescriptUtils.parseInteger(id) ?? 0;

  const [poll, setPoll] = useState<PollDetail | null>(null);
  const [loading, setLoading] = useState(true);
  const [refreshing, setRefreshing] = useState(false);
  const [voting, setVoting] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const fetchPoll = useCallback(async () => {
    try {
      setError(null);
      const data = await getPoll(pollID);
      setPoll(data);
    } catch (e: any) {
      setError(e.message || "Failed to load poll");
    } finally {
      setLoading(false);
      setRefreshing(false);
    }
  }, [pollID]);

  useFocusEffect(
    useCallback(() => {
      fetchPoll();
    }, [fetchPoll])
  );

  const handleVote = async (option: PollOption) => {
    setVoting(true);
    try {
      await submitVote(pollID, option.optionID);
      await fetchPoll();
    } catch (e: any) {
      Alert.alert("Vote failed", e.message || "Something went wrong");
    } finally {
      setVoting(false);
    }
  };

  if (loading) {
    return (
      <View style={commonStyles.centeredScreen}>
        <ActivityIndicator size="large" color={appColors.accent} />
      </View>
    );
  }

  if (error || !poll) {
    return (
      <View style={commonStyles.centeredScreen}>
        <Text style={commonStyles.errorText}>{error || "Poll not found"}</Text>
        <TouchableOpacity style={commonStyles.retryButton} onPress={fetchPoll}>
          <Text style={commonStyles.retryButtonText}>Retry</Text>
        </TouchableOpacity>
      </View>
    );
  }

  const totalVotes = TypescriptUtils.parseInteger(poll.totalVotes) ?? 0;

  return (
    <ScrollView
      style={commonStyles.screen}
      contentContainerStyle={commonStyles.screenContent}
      refreshControl={
        <RefreshControl
          refreshing={refreshing}
          onRefresh={() => {
            setRefreshing(true);
            fetchPoll();
          }}
        />
      }
    >
      <Text style={styles.question}>{poll.question}</Text>
      <Text style={styles.totalVotes}>
        {totalVotes} vote{totalVotes !== 1 ? "s" : ""} total
      </Text>

      {poll.options.map((option) => {
        const pct = totalVotes > 0 ? (option.votes / totalVotes) * 100 : 0;

        return (
          <TouchableOpacity
            key={option.optionID}
            style={[commonStyles.card, styles.optionCard]}
            onPress={() => handleVote(option)}
            disabled={voting}
            activeOpacity={0.7}
          >
            {/* Progress bar background */}
            <View
              style={[styles.progressBar, { width: `${pct}%` }]}
            />

            <View style={styles.optionContent}>
              <Text style={styles.optionText}>{option.text}</Text>
              <View style={styles.voteInfo}>
                <Text style={styles.voteCount}>{option.votes}</Text>
                {totalVotes > 0 && (
                  <Text style={styles.votePct}>{pct.toFixed(0)}%</Text>
                )}
              </View>
            </View>
          </TouchableOpacity>
        );
      })}

      {voting && (
        <View style={styles.votingOverlay}>
          <ActivityIndicator size="small" color={appColors.accent} />
          <Text style={styles.votingText}>Submitting vote...</Text>
        </View>
      )}
    </ScrollView>
  );
}

const styles = StyleSheet.create({
  question: {
    fontSize: 22,
    fontWeight: "bold",
    color: "#1a1a1a",
    marginBottom: 4,
  },
  totalVotes: { fontSize: 14, color: "#888", marginBottom: 20 },
  optionCard: {
    marginBottom: 10,
    overflow: "hidden",
    borderWidth: 1,
    borderColor: "#e0e0e0",
    position: "relative",
  },
  progressBar: {
    position: "absolute",
    top: 0,
    left: 0,
    bottom: 0,
    backgroundColor: appColors.accentSoft,
    borderRadius: 12,
  },
  optionContent: {
    flexDirection: "row",
    justifyContent: "space-between",
    alignItems: "center",
    padding: 16,
  },
  optionText: { fontSize: 16, fontWeight: "500", color: "#1a1a1a", flex: 1 },
  voteInfo: { flexDirection: "row", alignItems: "center", marginLeft: 12 },
  voteCount: { fontSize: 15, fontWeight: "bold", color: appColors.accent },
  votePct: { fontSize: 13, color: "#888", marginLeft: 6 },
  votingOverlay: {
    flexDirection: "row",
    justifyContent: "center",
    alignItems: "center",
    marginTop: 16,
  },
  votingText: { color: "#888", marginLeft: 8, fontSize: 14 },
});
