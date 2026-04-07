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
import { Redirect, useRouter, useFocusEffect } from "expo-router";
import { getPolls, PollSummary } from "../lib/api";
import { useAuth } from "../lib/auth";
import { appColors, commonStyles } from "../lib/styles";

export default function PollListScreen() {
  const { isAuthenticated } = useAuth();

  if (!isAuthenticated) {
    return <Redirect href="/login" />;
  }

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
      <View style={commonStyles.centeredScreen}>
        <ActivityIndicator size="large" color={appColors.accent} />
      </View>
    );
  }

  if (error) {
    return (
      <View style={commonStyles.centeredScreen}>
        <Text style={commonStyles.errorText}>{error}</Text>
        <TouchableOpacity style={commonStyles.retryButton} onPress={fetchPolls}>
          <Text style={commonStyles.retryButtonText}>Retry</Text>
        </TouchableOpacity>
      </View>
    );
  }

  return (
    <View style={commonStyles.screen}>
      <FlatList
        data={polls}
        keyExtractor={(item) => String(item.pollID)}
        refreshControl={
          <RefreshControl refreshing={refreshing} onRefresh={onRefresh} />
        }
        contentContainerStyle={polls.length === 0 ? styles.emptyState : undefined}
        ListEmptyComponent={
          <Text style={styles.emptyText}>No polls yet. Create one!</Text>
        }
        renderItem={({ item }) => (
          <TouchableOpacity
            style={[commonStyles.card, styles.pollCard]}
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
        style={commonStyles.floatingActionButton}
        onPress={() => router.push("/create")}
      >
        <Text style={commonStyles.floatingActionButtonText}>+</Text>
      </TouchableOpacity>
    </View>
  );
}

const styles = StyleSheet.create({
  emptyState: { flexGrow: 1, justifyContent: "center", alignItems: "center" },
  pollCard: {
    marginHorizontal: 16,
    marginTop: 12,
    padding: 16,
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
});
