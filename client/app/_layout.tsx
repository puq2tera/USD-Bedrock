import { Stack } from "expo-router";
import { StatusBar } from "expo-status-bar";
import { ActivityIndicator, View } from "react-native";
import { useEffect, useState } from "react";
import { hasToken, initializeAuth } from "../lib/api";

export default function RootLayout() {
  const [ready, setReady] = useState(false);

  useEffect(() => {
    initializeAuth().finally(() => setReady(true));
  }, []);

  if (!ready) {
    return (
      <View style={{ flex: 1, justifyContent: "center", alignItems: "center" }}>
        <ActivityIndicator size="large" color="#0D7E3F" />
      </View>
    );
  }

  const authenticated = hasToken();

  return (
    <>
      <StatusBar style="dark" />
      <Stack
        initialRouteName={authenticated ? "index" : "login"}
        screenOptions={{
          headerStyle: { backgroundColor: "#0D7E3F" },
          headerTintColor: "#fff",
          headerTitleStyle: { fontWeight: "bold" },
        }}
      >
        <Stack.Screen name="login" options={{ title: "Login", headerShown: false }} />
        <Stack.Screen name="register" options={{ title: "Register" }} />
        <Stack.Screen name="index" options={{ title: "Polls" }} />
        <Stack.Screen name="create" options={{ title: "Create Poll" }} />
        <Stack.Screen name="poll/[id]" options={{ title: "Poll" }} />
      </Stack>
    </>
  );
}
