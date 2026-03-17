import { useCallback, useState } from "react";
import {
  View,
  Text,
  FlatList,
  TouchableOpacity,
  StyleSheet,
  ActivityIndicator,
  RefreshControl,
} from "react-native";
import { useRouter, useFocusEffect } from "expo-router";
import { getPolls, PollSummary } from "../lib/api";

export default function PollListScreen() {
  const router = useRouter();
  const [polls, setPolls] = useState<PollSummary[]>([]);
  const [loading, setLoading] = useState(true);
  const [refreshing, setRefreshing] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const fetchPolls = useCallback(async () => {
    try {
      setError(null);
      const data = await getPolls();
      setPolls(data);
    } catch (e: any) {
      setError(e.message || "Failed to load polls");
    } finally {
      setLoading(false);
      setRefreshing(false);
    }
  }, []);

  // Refresh every time this screen comes into focus
  useFocusEffect(
    useCallback(() => {
      fetchPolls();
    }, [fetchPolls])
  );

  const onRefresh = () => {
    setRefreshing(true);
    fetchPolls();
  };

  if (loading) {
    return (
      <View style={styles.center}>
        <ActivityIndicator size="large" color="#0D7E3F" />
      </View>
    );
  }

  if (error) {
    return (
      <View style={styles.center}>
        <Text style={styles.errorText}>{error}</Text>
        <TouchableOpacity style={styles.retryButton} onPress={fetchPolls}>
          <Text style={styles.retryText}>Retry</Text>
        </TouchableOpacity>
      </View>
    );
  }

  return (
    <View style={styles.container}>
      <FlatList
        data={polls}
        keyExtractor={(item) => String(item.pollID)}
        refreshControl={
          <RefreshControl refreshing={refreshing} onRefresh={onRefresh} />
        }
        contentContainerStyle={polls.length === 0 ? styles.center : undefined}
        ListEmptyComponent={
          <Text style={styles.emptyText}>No polls yet. Create one!</Text>
        }
        renderItem={({ item }) => (
          <TouchableOpacity
            style={styles.pollCard}
            onPress={() => router.push(`/poll/${item.pollID}`)}
          >
            <Text style={styles.question}>{item.question}</Text>
            <View style={styles.meta}>
              <Text style={styles.metaText}>
                {item.optionCount} options
              </Text>
              <Text style={styles.metaText}>
                {item.totalVotes} vote{item.totalVotes !== 1 ? "s" : ""}
              </Text>
            </View>
          </TouchableOpacity>
        )}
      />

      <TouchableOpacity
        style={styles.fab}
        onPress={() => router.push("/create")}
      >
        <Text style={styles.fabText}>+</Text>
      </TouchableOpacity>
    </View>
  );
}

const styles = StyleSheet.create({
  container: { flex: 1, backgroundColor: "#f5f5f5" },
  center: { flex: 1, justifyContent: "center", alignItems: "center" },
  pollCard: {
    backgroundColor: "#fff",
    marginHorizontal: 16,
    marginTop: 12,
    padding: 16,
    borderRadius: 12,
    shadowColor: "#000",
    shadowOffset: { width: 0, height: 1 },
    shadowOpacity: 0.1,
    shadowRadius: 3,
    elevation: 2,
  },
  question: { fontSize: 17, fontWeight: "600", color: "#1a1a1a" },
  meta: {
    flexDirection: "row",
    justifyContent: "space-between",
    marginTop: 8,
  },
  metaText: { fontSize: 13, color: "#888" },
  emptyText: { fontSize: 16, color: "#999" },
  errorText: { fontSize: 16, color: "#d32f2f", marginBottom: 12 },
  retryButton: {
    backgroundColor: "#0D7E3F",
    paddingHorizontal: 24,
    paddingVertical: 10,
    borderRadius: 8,
  },
  retryText: { color: "#fff", fontWeight: "600" },
  fab: {
    position: "absolute",
    bottom: 32,
    right: 24,
    width: 56,
    height: 56,
    borderRadius: 28,
    backgroundColor: "#0D7E3F",
    justifyContent: "center",
    alignItems: "center",
    shadowColor: "#000",
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.25,
    shadowRadius: 4,
    elevation: 5,
  },
  fabText: { color: "#fff", fontSize: 28, lineHeight: 30 },
});
