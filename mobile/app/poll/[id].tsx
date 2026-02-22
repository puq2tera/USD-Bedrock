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
import { useLocalSearchParams } from "expo-router";
import { useFocusEffect } from "expo-router";
import { getPoll, submitVote, PollDetail, PollOption } from "../../lib/api";

export default function PollDetailScreen() {
  const { id } = useLocalSearchParams<{ id: string }>();
  const pollID = Number(id);

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
      <View style={styles.center}>
        <ActivityIndicator size="large" color="#0D7E3F" />
      </View>
    );
  }

  if (error || !poll) {
    return (
      <View style={styles.center}>
        <Text style={styles.errorText}>{error || "Poll not found"}</Text>
        <TouchableOpacity style={styles.retryButton} onPress={fetchPoll}>
          <Text style={styles.retryText}>Retry</Text>
        </TouchableOpacity>
      </View>
    );
  }

  const totalVotes = Number(poll.totalVotes);

  return (
    <ScrollView
      style={styles.container}
      contentContainerStyle={styles.scroll}
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
            style={styles.optionCard}
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
          <ActivityIndicator size="small" color="#0D7E3F" />
          <Text style={styles.votingText}>Submitting vote...</Text>
        </View>
      )}
    </ScrollView>
  );
}

const styles = StyleSheet.create({
  container: { flex: 1, backgroundColor: "#f5f5f5" },
  scroll: { padding: 16, paddingBottom: 40 },
  center: { flex: 1, justifyContent: "center", alignItems: "center" },
  question: {
    fontSize: 22,
    fontWeight: "bold",
    color: "#1a1a1a",
    marginBottom: 4,
  },
  totalVotes: { fontSize: 14, color: "#888", marginBottom: 20 },
  optionCard: {
    backgroundColor: "#fff",
    borderRadius: 12,
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
    backgroundColor: "rgba(13, 126, 63, 0.12)",
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
  voteCount: { fontSize: 15, fontWeight: "bold", color: "#0D7E3F" },
  votePct: { fontSize: 13, color: "#888", marginLeft: 6 },
  votingOverlay: {
    flexDirection: "row",
    justifyContent: "center",
    alignItems: "center",
    marginTop: 16,
  },
  votingText: { color: "#888", marginLeft: 8, fontSize: 14 },
  errorText: { fontSize: 16, color: "#d32f2f", marginBottom: 12 },
  retryButton: {
    backgroundColor: "#0D7E3F",
    paddingHorizontal: 24,
    paddingVertical: 10,
    borderRadius: 8,
  },
  retryText: { color: "#fff", fontWeight: "600" },
});
