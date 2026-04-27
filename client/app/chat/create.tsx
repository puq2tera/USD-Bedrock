import { useState } from "react";
import { Alert, KeyboardAvoidingView, Platform, StyleSheet, Text, TextInput, TouchableOpacity, View } from "react-native";
import { Redirect, useRouter } from "expo-router";
import { createChat } from "../../lib/api";
import { useAuth } from "../../lib/auth";
import { appColors, commonStyles } from "../../lib/styles";

export default function CreateChatScreen() {
  const { isAuthenticated } = useAuth();
  const router = useRouter();
  const [title, setTitle] = useState("");
  const [submitting, setSubmitting] = useState(false);

  if (!isAuthenticated) {
    return <Redirect href="/login" />;
  }

  const handleCreate = async () => {
    if (!title.trim()) {
      Alert.alert("Missing title", "Please enter a chat title.");
      return;
    }

    setSubmitting(true);
    try {
      const created = await createChat(title.trim());
      router.replace(`/chat/${created.chatID}`);
    } catch (e: any) {
      Alert.alert("Unable to create chat", e?.message || "Please try again.");
    } finally {
      setSubmitting(false);
    }
  };

  return (
    <KeyboardAvoidingView style={commonStyles.screen} behavior={Platform.OS === "ios" ? "padding" : undefined}>
      <View style={commonStyles.screenContent}>
        <Text style={styles.pageTitle}>Create Chat</Text>
        <Text style={styles.pageSubtitle}>Start a focused thread for your team, project, or topic.</Text>

        <Text style={commonStyles.sectionLabel}>Chat title</Text>
        <TextInput
          style={commonStyles.input}
          value={title}
          onChangeText={setTitle}
          placeholder="Team discussion"
          placeholderTextColor={appColors.textSubtle}
          maxLength={255}
        />
        <TouchableOpacity
          style={[commonStyles.primaryButtonLarge, styles.submitButton, submitting && commonStyles.primaryButtonDisabled]}
          disabled={submitting}
          onPress={handleCreate}
        >
          <Text style={commonStyles.primaryButtonLargeText}>{submitting ? "Creating..." : "Create Chat"}</Text>
        </TouchableOpacity>
      </View>
    </KeyboardAvoidingView>
  );
}

const styles = StyleSheet.create({
  pageTitle: {
    color: appColors.text,
    fontSize: 24,
    fontWeight: "700",
  },
  pageSubtitle: {
    color: appColors.textMuted,
    marginTop: 6,
    fontSize: 13,
    lineHeight: 18,
  },
  submitButton: {
    marginTop: 24,
  },
});
