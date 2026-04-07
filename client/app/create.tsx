import { useState } from "react";
import {
  View,
  Text,
  TextInput,
  TouchableOpacity,
  StyleSheet,
  ScrollView,
  Alert,
  KeyboardAvoidingView,
  Platform,
} from "react-native";
import { useRouter } from "expo-router";
import { Redirect } from "expo-router";
import { createPoll } from "../lib/api";
import { useAuth } from "../lib/auth";
import { appColors, appSpacing, commonStyles } from "../lib/styles";
import TypescriptUtils from "../lib/TypescriptUtils";

export default function CreatePollScreen() {
  const { isAuthenticated } = useAuth();

  if (!isAuthenticated) {
    return <Redirect href="/login" />;
  }

  const router = useRouter();
  const [question, setQuestion] = useState("");
  const [options, setOptions] = useState(["", ""]);
  const [submitting, setSubmitting] = useState(false);

  const addOption = () => {
    if (options.length >= 20) {
      Alert.alert("Limit reached", "A poll can have at most 20 options.");
      return;
    }
    setOptions([...options, ""]);
  };

  const removeOption = (index: number) => {
    if (options.length <= 2) {
      Alert.alert("Minimum options", "A poll needs at least 2 options.");
      return;
    }
    setOptions(options.filter((_, i) => i !== index));
  };

  const updateOption = (index: number, text: string) => {
    const updated = TypescriptUtils.clone(options);
    updated[index] = text;
    setOptions(updated);
  };

  const handleSubmit = async () => {
    const trimmedQuestion = question.trim();
    const trimmedOptions = options.map((o) => o.trim()).filter((o) => o.length > 0);
    const uniqueOptions = TypescriptUtils.dedupe<string>(trimmedOptions);

    if (TypescriptUtils.isNullOrWhiteSpace(trimmedQuestion)) {
      Alert.alert("Missing question", "Please enter a question for your poll.");
      return;
    }

    if (trimmedOptions.length < 2) {
      Alert.alert("Not enough options", "A poll needs at least 2 non-empty options.");
      return;
    }

    if (uniqueOptions.length !== trimmedOptions.length) {
      Alert.alert("Duplicate options", "Each option must be unique.");
      return;
    }

    setSubmitting(true);
    try {
      const result = await createPoll(trimmedQuestion, uniqueOptions);
      router.replace(`/poll/${result.pollID}`);
    } catch (e: any) {
      Alert.alert("Error", e.message || "Failed to create poll");
    } finally {
      setSubmitting(false);
    }
  };

  return (
    <KeyboardAvoidingView
      style={commonStyles.screen}
      behavior={Platform.OS === "ios" ? "padding" : undefined}
    >
      <ScrollView contentContainerStyle={commonStyles.screenContent}>
        <Text style={commonStyles.sectionLabel}>Question</Text>
        <TextInput
          style={styles.questionInput}
          placeholder="What do you want to ask?"
          value={question}
          onChangeText={setQuestion}
          multiline
        />

        <Text style={commonStyles.sectionLabel}>Options</Text>
        {options.map((opt, index) => (
          <View key={index} style={styles.optionRow}>
            <TextInput
              style={styles.optionInput}
              placeholder={`Option ${index + 1}`}
              value={opt}
              onChangeText={(text) => updateOption(index, text)}
            />
            <TouchableOpacity
              style={styles.removeButton}
              onPress={() => removeOption(index)}
            >
              <Text style={styles.removeText}>X</Text>
            </TouchableOpacity>
          </View>
        ))}

        <TouchableOpacity style={styles.addButton} onPress={addOption}>
          <Text style={[commonStyles.textLink, styles.addButtonText]}>+ Add Option</Text>
        </TouchableOpacity>

        <TouchableOpacity
          style={[commonStyles.primaryButtonLarge, submitting && commonStyles.primaryButtonDisabled]}
          onPress={handleSubmit}
          disabled={submitting}
        >
          <Text style={commonStyles.primaryButtonLargeText}>
            {submitting ? "Creating..." : "Create Poll"}
          </Text>
        </TouchableOpacity>
      </ScrollView>
    </KeyboardAvoidingView>
  );
}

const styles = StyleSheet.create({
  questionInput: {
    ...commonStyles.input,
    paddingVertical: appSpacing.inputInset,
    minHeight: 96,
    textAlignVertical: "top",
  },
  optionRow: {
    flexDirection: "row",
    alignItems: "center",
    marginBottom: 8,
  },
  optionInput: {
    ...commonStyles.input,
    flex: 1,
  },
  removeButton: {
    marginLeft: 8,
    width: 36,
    height: 36,
    borderRadius: 18,
    backgroundColor: appColors.neutralButton,
    justifyContent: "center",
    alignItems: "center",
  },
  removeText: { color: appColors.neutralButtonText, fontWeight: "bold", fontSize: 14 },
  addButton: {
    paddingVertical: 12,
    alignItems: "center",
    marginTop: 4,
  },
  addButtonText: { fontSize: 15 },
});
