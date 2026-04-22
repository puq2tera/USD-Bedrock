import { Stack } from "expo-router";
import { StatusBar } from "expo-status-bar";
import { ActivityIndicator, Text, TouchableOpacity, View } from "react-native";
import { useEffect } from "react";
import { useRouter, useSegments } from "expo-router";
import { AuthProvider, useAuth } from "../lib/auth";
import { appColors, appStackScreenOptions, commonStyles } from "../lib/styles";

function AccountIconButton() {
  const router = useRouter();

  return (
    <TouchableOpacity style={commonStyles.circularIconButton} onPress={() => router.push("/account")}>
      <Text style={commonStyles.circularIconButtonText}>👤</Text>
    </TouchableOpacity>
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
        screenOptions={({ navigation }) => ({
          ...appStackScreenOptions,
          headerRight: () => (isAuthenticated ? <AccountIconButton /> : null),
          headerLeft: navigation.canGoBack()
            ? () => (
                <TouchableOpacity style={commonStyles.circularIconButton} onPress={() => navigation.goBack()}>
                  <Text style={commonStyles.circularIconButtonText}>←</Text>
                </TouchableOpacity>
              )
            : undefined,
        })}
      >
        <Stack.Screen name="login" options={{ title: "Login", headerShown: false }} />
        <Stack.Screen name="register" options={{ title: "Register" }} />
        <Stack.Screen name="index" options={{ title: "Chats" }} />
        <Stack.Screen name="chat/create" options={{ title: "Create Chat" }} />
        <Stack.Screen name="chat/[id]" options={{ title: "Chat" }} />
        <Stack.Screen name="chat/[id]/settings" options={{ title: "Chat Settings" }} />
        <Stack.Screen name="chat/[id]/poll/create" options={{ title: "Create Poll" }} />
        <Stack.Screen name="chat/[id]/poll/[pollId]" options={{ title: "Poll" }} />
        <Stack.Screen name="chat/[id]/poll/[pollId]/settings" options={{ title: "Poll Settings" }} />
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
