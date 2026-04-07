import { Stack } from "expo-router";
import { StatusBar } from "expo-status-bar";
import { ActivityIndicator, View } from "react-native";
import { useEffect } from "react";
import { useRouter, useSegments } from "expo-router";
import { AuthProvider, useAuth } from "../lib/auth";
import { appColors, appStackScreenOptions, commonStyles } from "../lib/styles";

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
      <Stack screenOptions={appStackScreenOptions}>
        <Stack.Screen name="login" options={{ title: "Login", headerShown: false }} />
        <Stack.Screen name="register" options={{ title: "Register" }} />
        <Stack.Screen name="index" options={{ title: "Polls" }} />
        <Stack.Screen name="create" options={{ title: "Create Poll" }} />
        <Stack.Screen name="poll/[id]" options={{ title: "Poll" }} />
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
