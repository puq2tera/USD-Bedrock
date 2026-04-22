import { Alert, ScrollView, StyleSheet, Text, TouchableOpacity, View } from "react-native";
import { Redirect, useFocusEffect, useRouter } from "expo-router";
import { useCallback, useState } from "react";
import { getAccount } from "../lib/api";
import { useAuth } from "../lib/auth";
import { commonStyles } from "../lib/styles";

export default function AccountScreen() {
  const { isAuthenticated, user, logout } = useAuth();
  const router = useRouter();
  const [email, setEmail] = useState(user?.email ?? "");
  const [createdAt, setCreatedAt] = useState("");

  useFocusEffect(
    useCallback(() => {
      if (!isAuthenticated) {
        return;
      }

      void (async () => {
        try {
          const account = await getAccount();
          setEmail(account.email);
          setCreatedAt(account.createdAt);
        } catch {
          // Leave last known profile values if refresh fails.
        }
      })();
    }, [isAuthenticated])
  );

  if (!isAuthenticated) {
    return <Redirect href="/login" />;
  }

  return (
    <ScrollView style={commonStyles.screen} contentContainerStyle={commonStyles.screenContent}>
      <View style={[commonStyles.sectionCard, styles.accountCard]}>
        <Text style={commonStyles.pageTitle}>{user?.displayName || "Account"}</Text>
        <Text style={[commonStyles.metaText, styles.metaPrimary]}>Email: {email || user?.email || ""}</Text>
        <Text style={[commonStyles.metaText, styles.metaSecondary]}>User ID: {user?.userID || ""}</Text>
        <Text style={[commonStyles.metaText, styles.metaSecondary]}>Created at: {createdAt || "Unknown"}</Text>
      </View>

      <TouchableOpacity
        style={[commonStyles.primaryButtonLarge, styles.signOutButton]}
        onPress={async () => {
          await logout();
          router.replace("/login");
        }}
      >
        <Text style={commonStyles.primaryButtonLargeText}>Sign out</Text>
      </TouchableOpacity>

      <TouchableOpacity style={[commonStyles.retryButton, styles.settingsButton]} onPress={() => router.push("/settings")}>
        <Text style={commonStyles.retryButtonText}>Open settings</Text>
      </TouchableOpacity>
    </ScrollView>
  );
}

const styles = StyleSheet.create({
  accountCard: {
    padding: 16,
  },
  metaPrimary: {
    marginTop: 6,
  },
  metaSecondary: {
    marginTop: 4,
  },
  signOutButton: {
    marginTop: 18,
  },
  settingsButton: {
    marginTop: 14,
    alignSelf: "flex-start",
  },
});
