import { Stack } from "expo-router";
import { StatusBar } from "expo-status-bar";
import { ActivityIndicator, Text, TouchableOpacity, View } from "react-native";
import { useEffect } from "react";
import { useRouter, useSegments } from "expo-router";
import { AuthProvider, useAuth } from "../lib/auth";
import { appColors, appStackScreenOptions, commonStyles } from "../lib/styles";

function HeaderActions() {
  const router = useRouter();

  return (
    <View style={commonStyles.headerActionsRow}>
      <TouchableOpacity onPress={() => router.push("/account")}>
        <Text style={commonStyles.headerActionText}>Account</Text>
      </TouchableOpacity>
      <TouchableOpacity onPress={() => router.push("/settings")}>
        <Text style={commonStyles.headerActionText}>Settings</Text>
      </TouchableOpacity>
    </View>
  );
}

function RootNavigator() {
  const router = useRouter();
  const segments = useSegments();
  const { status, isAuthenticated } = useAuth();

  useEffect(() => {
    const inAuthFlow = segments[0] === "login" || segments[0] === "register";
    if (status === "loading") {
      return;
    }

    if (!isAuthenticated && !inAuthFlow) {
      router.replace("/login");
      return;
    }

    if (isAuthenticated && inAuthFlow) {
      router.replace("/");
    }
  }, [isAuthenticated, router, segments, status]);

  if (status === "loading") {
    return (
      <View style={commonStyles.centeredScreen}>
        <ActivityIndicator size="large" color={appColors.accent} />
      </View>
    );
  }

  return (
    <>
      <StatusBar style="dark" />
      <Stack
        screenOptions={{
          ...appStackScreenOptions,
          headerRight: () => (isAuthenticated ? <HeaderActions /> : null),
        }}
      >
        <Stack.Screen name="login" options={{ title: "Login", headerShown: false }} />
        <Stack.Screen name="register" options={{ title: "Register" }} />
        <Stack.Screen name="index" options={{ title: "Chats" }} />
        <Stack.Screen name="chat/create" options={{ title: "Create Chat" }} />
        <Stack.Screen name="chat/[id]" options={{ title: "Chat" }} />
        <Stack.Screen name="chat/[id]/poll/create" options={{ title: "Create Poll" }} />
        <Stack.Screen name="chat/[id]/poll/[pollId]" options={{ title: "Poll" }} />
        <Stack.Screen name="account" options={{ title: "Account" }} />
        <Stack.Screen name="settings" options={{ title: "Settings" }} />
      </Stack>
    </>
  );
}

export default function RootLayout() {
  return (
    <AuthProvider>
      <RootNavigator />
    </AuthProvider>
  );
}
