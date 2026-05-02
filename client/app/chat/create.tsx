import { useState } from "react";
import { KeyboardAvoidingView, Platform, StyleSheet, Text, TextInput, TouchableOpacity, View } from "react-native";
import { Redirect, useRouter } from "expo-router";
import { createChat } from "../../lib/api";
import { useAuth } from "../../lib/auth";
import { appColors, commonStyles } from "../../lib/styles";

export default function CreateChatScreen() {
  const { isAuthenticated } = useAuth();
  const router = useRouter();
  const [title, setTitle] = useState("");
  const [submitting, setSubmitting] = useState(false);
  const [titleError, setTitleError] = useState<string | null>(null);
  const [formError, setFormError] = useState<string | null>(null);

  if (!isAuthenticated) {
    return <Redirect href="/login" />;
  }

  const handleCreate = async () => {
    setTitleError(null);
    setFormError(null);
    if (!title.trim()) {
      setTitleError("Enter a chat title.");
      return;
    }

    setSubmitting(true);
    try {
      const created = await createChat(title.trim());
      router.replace(`/chat/${created.chatID}`);
    } catch (e: any) {
      setFormError(e?.message || "Unable to create chat. Please try again.");
    } finally {
      setSubmitting(false);
    }
  };

  return (
    <KeyboardAvoidingView style={commonStyles.screen} behavior={Platform.OS === "ios" ? "padding" : undefined}>
      <View style={commonStyles.screenContent}>
        <Text style={commonStyles.pageTitle}>Create Chat</Text>
        <Text style={commonStyles.bodyTextMuted}>Start a focused thread for your team, project, or topic.</Text>

        <Text style={commonStyles.sectionLabel}>Chat title</Text>
        <TextInput
          style={commonStyles.input}
          value={title}
          onChangeText={(nextTitle) => {
            setTitle(nextTitle);
            setTitleError(null);
            setFormError(null);
          }}
          placeholder="Team discussion"
          placeholderTextColor={appColors.textSubtle}
          maxLength={255}
        />
        {titleError && <Text style={commonStyles.errorText}>{titleError}</Text>}
        {formError && (
          <View style={[commonStyles.feedbackBanner, commonStyles.errorBanner]}>
            <Text style={commonStyles.errorBannerText}>{formError}</Text>
          </View>
        )}
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
  submitButton: {
    marginTop: 24,
  },
});
