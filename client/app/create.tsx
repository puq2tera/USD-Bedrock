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
import { createPoll } from "../lib/api";

export default function CreatePollScreen() {
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
    const updated = [...options];
    updated[index] = text;
    setOptions(updated);
  };

  const handleSubmit = async () => {
    const trimmedQuestion = question.trim();
    const trimmedOptions = options.map((o) => o.trim()).filter((o) => o.length > 0);

    if (!trimmedQuestion) {
      Alert.alert("Missing question", "Please enter a question for your poll.");
      return;
    }

    if (trimmedOptions.length < 2) {
      Alert.alert("Not enough options", "A poll needs at least 2 non-empty options.");
      return;
    }

    const uniqueOptions = new Set(trimmedOptions);
    if (uniqueOptions.size !== trimmedOptions.length) {
      Alert.alert("Duplicate options", "Each option must be unique.");
      return;
    }

    setSubmitting(true);
    try {
      const result = await createPoll(trimmedQuestion, trimmedOptions);
      router.replace(`/poll/${result.pollID}`);
    } catch (e: any) {
      Alert.alert("Error", e.message || "Failed to create poll");
    } finally {
      setSubmitting(false);
    }
  };

  return (
    <KeyboardAvoidingView
      style={styles.container}
      behavior={Platform.OS === "ios" ? "padding" : undefined}
    >
      <ScrollView contentContainerStyle={styles.scroll}>
        <Text style={styles.label}>Question</Text>
        <TextInput
          style={styles.input}
          placeholder="What do you want to ask?"
          value={question}
          onChangeText={setQuestion}
          multiline
        />

        <Text style={styles.label}>Options</Text>
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
          <Text style={styles.addButtonText}>+ Add Option</Text>
        </TouchableOpacity>

        <TouchableOpacity
          style={[styles.submitButton, submitting && styles.submitDisabled]}
          onPress={handleSubmit}
          disabled={submitting}
        >
          <Text style={styles.submitText}>
            {submitting ? "Creating..." : "Create Poll"}
          </Text>
        </TouchableOpacity>
      </ScrollView>
    </KeyboardAvoidingView>
  );
}

const styles = StyleSheet.create({
  container: { flex: 1, backgroundColor: "#f5f5f5" },
  scroll: { padding: 16, paddingBottom: 40 },
  label: {
    fontSize: 15,
    fontWeight: "600",
    color: "#555",
    marginBottom: 6,
    marginTop: 16,
  },
  input: {
    backgroundColor: "#fff",
    borderRadius: 10,
    padding: 14,
    fontSize: 16,
    borderWidth: 1,
    borderColor: "#ddd",
  },
  optionRow: {
    flexDirection: "row",
    alignItems: "center",
    marginBottom: 8,
  },
  optionInput: {
    flex: 1,
    backgroundColor: "#fff",
    borderRadius: 10,
    padding: 14,
    fontSize: 16,
    borderWidth: 1,
    borderColor: "#ddd",
  },
  removeButton: {
    marginLeft: 8,
    width: 36,
    height: 36,
    borderRadius: 18,
    backgroundColor: "#eee",
    justifyContent: "center",
    alignItems: "center",
  },
  removeText: { color: "#999", fontWeight: "bold", fontSize: 14 },
  addButton: {
    paddingVertical: 12,
    alignItems: "center",
    marginTop: 4,
  },
  addButtonText: { color: "#0D7E3F", fontWeight: "600", fontSize: 15 },
  submitButton: {
    backgroundColor: "#0D7E3F",
    paddingVertical: 16,
    borderRadius: 12,
    alignItems: "center",
    marginTop: 24,
  },
  submitDisabled: { opacity: 0.6 },
  submitText: { color: "#fff", fontSize: 17, fontWeight: "bold" },
});
