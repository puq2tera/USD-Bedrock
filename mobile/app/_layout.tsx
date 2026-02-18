import { Stack } from "expo-router";
import { StatusBar } from "expo-status-bar";

export default function RootLayout() {
  return (
    <>
      <StatusBar style="dark" />
      <Stack
        screenOptions={{
          headerStyle: { backgroundColor: "#0D7E3F" },
          headerTintColor: "#fff",
          headerTitleStyle: { fontWeight: "bold" },
        }}
      >
        <Stack.Screen name="index" options={{ title: "Polls" }} />
        <Stack.Screen name="create" options={{ title: "Create Poll" }} />
        <Stack.Screen name="poll/[id]" options={{ title: "Poll" }} />
      </Stack>
    </>
  );
}
