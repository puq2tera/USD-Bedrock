import { useCallback, useState } from "react";
import {
  ActivityIndicator,
  FlatList,
  RefreshControl,
  StyleSheet,
  Text,
  TouchableOpacity,
  View,
} from "react-native";
import { Redirect, useFocusEffect, useRouter } from "expo-router";
import { ChatSummary, getIdentityLabel, listChats } from "../lib/api";
import { useAuth } from "../lib/auth";
import { appColors, commonStyles } from "../lib/styles";

export default function ChatListScreen() {
  const { isAuthenticated } = useAuth();
  const router = useRouter();
  const [chats, setChats] = useState<ChatSummary[]>([]);
  const [loading, setLoading] = useState(true);
  const [refreshing, setRefreshing] = useState(false);
  const [loadingMore, setLoadingMore] = useState(false);
  const [nextBeforeChatID, setNextBeforeChatID] = useState<string | null>(null);
  const [error, setError] = useState<string | null>(null);

  const fetchChats = useCallback(async () => {
    try {
      setError(null);
      const firstPage = await listChats();
      setChats(firstPage);
      setNextBeforeChatID(firstPage.length > 0 ? firstPage[firstPage.length - 1].chatID : null);
    } catch (e: any) {
      setError(e?.message || "Failed to load chats");
    } finally {
      setLoading(false);
      setRefreshing(false);
    }
  }, []);

  const fetchMore = useCallback(async () => {
    if (loadingMore || !nextBeforeChatID) {
      return;
    }

    setLoadingMore(true);
    try {
      const nextPage = await listChats(nextBeforeChatID);
      if (nextPage.length < 1) {
        setNextBeforeChatID(null);
        return;
      }

      setChats((prev) => [...prev, ...nextPage]);
      setNextBeforeChatID(nextPage[nextPage.length - 1].chatID);
    } catch {
      // Keep list usable if pagination fails; users can pull to refresh.
      setNextBeforeChatID(null);
    } finally {
      setLoadingMore(false);
    }
  }, [loadingMore, nextBeforeChatID]);

  useFocusEffect(
    useCallback(() => {
      fetchChats();
    }, [fetchChats])
  );

  if (!isAuthenticated) {
    return <Redirect href="/login" />;
  }

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
        <TouchableOpacity style={commonStyles.retryButton} onPress={fetchChats}>
          <Text style={commonStyles.retryButtonText}>Retry</Text>
        </TouchableOpacity>
      </View>
    );
  }

  return (
    <View style={commonStyles.screen}>
      <View style={styles.hero}>
        <View>
          <Text style={styles.heroTitle}>Chats</Text>
          <Text style={styles.heroSubtitle}>Pick up where your team left off.</Text>
        </View>
        <TouchableOpacity style={commonStyles.primaryButton} onPress={() => router.push("/chat/create")}>
          <Text style={commonStyles.primaryButtonText}>New Chat</Text>
        </TouchableOpacity>
      </View>

      <FlatList
        data={chats}
        keyExtractor={(chat) => chat.chatID}
        refreshControl={<RefreshControl refreshing={refreshing} onRefresh={() => {
          setRefreshing(true);
          fetchChats();
        }} />}
        onEndReachedThreshold={0.5}
        onEndReached={fetchMore}
        ListFooterComponent={loadingMore ? <ActivityIndicator color={appColors.accent} style={styles.loader} /> : null}
        ListEmptyComponent={
          <View style={styles.emptyState}>
            <Text style={styles.emptyTitle}>No chats yet</Text>
            <Text style={styles.emptyText}>Create your first chat to start conversations and polls.</Text>
            <TouchableOpacity style={[commonStyles.primaryButton, styles.emptyAction]} onPress={() => router.push("/chat/create")}>
              <Text style={commonStyles.primaryButtonText}>Create Chat</Text>
            </TouchableOpacity>
          </View>
        }
        contentContainerStyle={chats.length < 1 ? styles.emptyContainer : undefined}
        renderItem={({ item }) => (
          <TouchableOpacity
            style={[commonStyles.card, styles.chatCard]}
            onPress={() => router.push(`/chat/${item.chatID}`)}
          >
            <Text style={styles.chatTitle}>{item.title}</Text>
            <Text style={styles.chatMeta}>by {getIdentityLabel(item.createdByUserID)}</Text>
            <View style={styles.roleChip}>
              <Text style={styles.roleChipText}>{item.requesterRole === "owner" ? "Owner" : "Member"}</Text>
            </View>
          </TouchableOpacity>
        )}
      />
    </View>
  );
}

const styles = StyleSheet.create({
  hero: {
    paddingHorizontal: 16,
    paddingTop: 16,
    paddingBottom: 12,
    gap: 10,
  },
  heroTitle: {
    fontSize: 24,
    fontWeight: "700",
    color: appColors.text,
  },
  heroSubtitle: {
    marginTop: 4,
    color: appColors.textMuted,
    fontSize: 13,
  },
  chatCard: {
    marginHorizontal: 16,
    marginTop: 12,
    padding: 16,
    borderWidth: 1,
    borderColor: appColors.borderSoft,
  },
  chatTitle: {
    fontSize: 18,
    fontWeight: "700",
    color: appColors.text,
    marginBottom: 6,
  },
  chatMeta: {
    fontSize: 13,
    color: appColors.textMuted,
  },
  roleChip: {
    marginTop: 10,
    alignSelf: "flex-start",
    backgroundColor: appColors.accentSoft,
    borderWidth: 1,
    borderColor: appColors.border,
    borderRadius: 12,
    paddingHorizontal: 10,
    paddingVertical: 4,
  },
  roleChipText: {
    color: appColors.textSubtle,
    fontWeight: "600",
    fontSize: 12,
  },
  emptyContainer: {
    flexGrow: 1,
    justifyContent: "center",
    alignItems: "center",
    paddingHorizontal: 20,
  },
  emptyState: {
    alignItems: "center",
  },
  emptyTitle: {
    color: appColors.text,
    fontSize: 18,
    fontWeight: "700",
    marginBottom: 6,
  },
  emptyText: {
    color: appColors.textMuted,
    fontSize: 14,
    textAlign: "center",
    lineHeight: 20,
  },
  emptyAction: {
    marginTop: 14,
    paddingHorizontal: 20,
  },
  loader: {
    marginVertical: 16,
  },
});
