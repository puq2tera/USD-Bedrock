import { useMemo, useRef, useState } from "react";
import {
  KeyboardAvoidingView,
  PanResponder,
  Platform,
  StyleSheet,
  Switch,
  Text,
  TextInput,
  TouchableOpacity,
  View,
} from "react-native";
import { Redirect, useLocalSearchParams, useRouter } from "expo-router";
import { KeyboardAwareScrollView } from "../../../../components/KeyboardAwareScrollView";
import { CreatePollInput, PollType, createPoll } from "../../../../lib/api";
import { parseDateTime } from "../../../../lib/dateTime";
import TypescriptUtils from "../../../../lib/TypescriptUtils";
import { useAuth } from "../../../../lib/auth";
import { appColors, commonStyles } from "../../../../lib/styles";

type PollTypeOption = {
  type: PollType;
  label: string;
  description: string;
};

const POLL_TYPES: PollTypeOption[] = [
  { type: "single_choice", label: "Single choice", description: "Everyone picks one option." },
  { type: "multiple_choice", label: "Multiple choice", description: "People can choose more than one option." },
  { type: "ranked_choice", label: "Ranked choice", description: "People rank options from best to worst." },
  { type: "free_text", label: "Text response only", description: "People submit an open-ended answer." },
];

export default function CreateChatPollScreen() {
  const { isAuthenticated } = useAuth();
  const router = useRouter();
  const { id } = useLocalSearchParams<{ id: string }>();
  const chatID = TypescriptUtils.parseString(id) ?? "";

  const [question, setQuestion] = useState("");
  const [type, setType] = useState<PollType>("single_choice");
  const [allowChangeVote, setAllowChangeVote] = useState(true);
  const [isAnonymous, setIsAnonymous] = useState(false);
  const [expiresAtInput, setExpiresAtInput] = useState("");
  const [options, setOptions] = useState<string[]>(["", ""]);
  const [submitting, setSubmitting] = useState(false);
  const [formError, setFormError] = useState<string | null>(null);

  const dragStateRef = useRef<{ startIndex: number; currentIndex: number; startPageY: number } | null>(null);
  const [draggingIndex, setDraggingIndex] = useState<number | null>(null);

  const selectedType = useMemo(() => POLL_TYPES.find((pollType) => pollType.type === type) ?? POLL_TYPES[0], [type]);
  const requiresOptions = useMemo(() => type !== "free_text", [type]);
  const isRankedChoice = type === "ranked_choice";

  if (!isAuthenticated) {
    return <Redirect href="/login" />;
  }

  const reorderOption = (from: number, to: number) => {
    setOptions((prev) => {
      if (to < 0 || to >= prev.length || from === to) {
        return prev;
      }

      const next = [...prev];
      const [value] = next.splice(from, 1);
      next.splice(to, 0, value);
      return next;
    });
  };

  const moveOption = (from: number, to: number) => {
    reorderOption(from, to);
  };

  const startDrag = (index: number, pageY: number) => {
    dragStateRef.current = {
      startIndex: index,
      currentIndex: index,
      startPageY: pageY,
    };
    setDraggingIndex(index);
  };

  const updateDrag = (pageY: number) => {
    if (!dragStateRef.current || !isRankedChoice) {
      return;
    }

    const rowHeightApproximation = 50;
    const deltaY = pageY - dragStateRef.current.startPageY;
    const deltaRows = Math.round(deltaY / rowHeightApproximation);
    const targetIndex = Math.max(0, Math.min(options.length - 1, dragStateRef.current.startIndex + deltaRows));

    if (targetIndex === dragStateRef.current.currentIndex) {
      return;
    }

    reorderOption(dragStateRef.current.currentIndex, targetIndex);
    dragStateRef.current.currentIndex = targetIndex;
    setDraggingIndex(targetIndex);
  };

  const endDrag = () => {
    dragStateRef.current = null;
    setDraggingIndex(null);
  };

  const submit = async () => {
    setFormError(null);
    if (TypescriptUtils.isNullOrWhiteSpace(chatID)) {
      setFormError("Invalid chat context. Return to the chat and try again.");
      return;
    }

    const normalizedQuestion = question.trim();
    if (!normalizedQuestion) {
      setFormError("Enter a poll question.");
      return;
    }

    const normalizedOptions = options.map((option) => option.trim()).filter((option) => option.length > 0);
    const dedupedOptions = TypescriptUtils.dedupe(normalizedOptions);
    if (requiresOptions && dedupedOptions.length < 2) {
      setFormError("This poll type requires at least two options.");
      return;
    }

    if (requiresOptions && dedupedOptions.length !== normalizedOptions.length) {
      setFormError("Options must be unique.");
      return;
    }

    let expiresAt: number | undefined;
    const trimmedExpiresAt = expiresAtInput.trim();
    if (trimmedExpiresAt.length > 0) {
      const parsedDate = parseDateTime(trimmedExpiresAt);
      if (parsedDate == null) {
        setFormError("Use a valid expiration datetime like 2026-05-01T18:00.");
        return;
      }

      expiresAt = Math.floor(parsedDate.getTime() / 1000);
    }

    const payload: CreatePollInput = {
      question: normalizedQuestion,
      type,
      allowChangeVote,
      isAnonymous,
      ...(expiresAt ? { expiresAt } : {}),
      ...(requiresOptions ? { options: dedupedOptions } : {}),
    };

    setSubmitting(true);
    try {
      const created = await createPoll(chatID, payload);
      router.replace(`/chat/${chatID}/poll/${created.pollID}`);
    } catch (e: any) {
      setFormError(e?.message || "Could not create poll. Please retry.");
    } finally {
      setSubmitting(false);
    }
  };

  return (
    <KeyboardAvoidingView style={commonStyles.screen} behavior={Platform.OS === "ios" ? "padding" : undefined}>
      <KeyboardAwareScrollView contentContainerStyle={commonStyles.screenContent}>
        <Text style={commonStyles.pageTitle}>Create Poll</Text>
        <Text style={commonStyles.bodyTextMuted}>Choose a poll format, tune options, and launch it in this chat.</Text>

        <Text style={commonStyles.sectionLabel}>Question</Text>
        <TextInput
          style={commonStyles.input}
          multiline
          value={question}
          onChangeText={setQuestion}
          placeholder="What should we decide?"
          placeholderTextColor={appColors.textSubtle}
        />

        <Text style={commonStyles.sectionLabel}>Type</Text>
        <View style={styles.segmentedContainer}>
          {POLL_TYPES.map((pollType) => (
            <TouchableOpacity
              key={pollType.type}
              style={[styles.segment, type === pollType.type && styles.segmentActive]}
              onPress={() => setType(pollType.type)}
            >
              <Text style={[styles.segmentLabel, type === pollType.type && styles.segmentLabelActive]}>{pollType.label}</Text>
            </TouchableOpacity>
          ))}
        </View>
        <Text style={styles.typeDescription}>{selectedType.description}</Text>

        <View style={commonStyles.formSwitchRow}>
          <Text style={commonStyles.formSwitchLabel}>Allow vote changes</Text>
          <Switch value={allowChangeVote} onValueChange={setAllowChangeVote} />
        </View>

        <View style={commonStyles.formSwitchRow}>
          <Text style={commonStyles.formSwitchLabel}>Anonymous</Text>
          <Switch value={isAnonymous} onValueChange={setIsAnonymous} />
        </View>

        <Text style={commonStyles.sectionLabel}>Expires at (optional)</Text>
        <TextInput
          style={commonStyles.input}
          value={expiresAtInput}
          onChangeText={setExpiresAtInput}
          placeholder="2026-05-01T18:00"
          placeholderTextColor={appColors.textSubtle}
          autoCapitalize="none"
        />

        {requiresOptions && (
          <>
            <Text style={commonStyles.sectionLabel}>Options</Text>
            {isRankedChoice && <Text style={styles.hint}>Drag an option row directly to reorder ranking priority, or use arrows.</Text>}
            {options.map((option, index) => (
              <View
                key={`option-${index}`}
                style={[styles.optionRow, draggingIndex === index && styles.optionRowDragging]}
                {...(isRankedChoice
                  ? PanResponder.create({
                      onStartShouldSetPanResponder: () => true,
                      onMoveShouldSetPanResponder: (_, gestureState) => Math.abs(gestureState.dy) > 4,
                      onPanResponderGrant: (event) => startDrag(index, event.nativeEvent.pageY),
                      onPanResponderMove: (event) => updateDrag(event.nativeEvent.pageY),
                      onPanResponderRelease: endDrag,
                      onPanResponderTerminate: endDrag,
                    }).panHandlers
                  : {})}
              >
                <TextInput
                  style={[commonStyles.input, styles.optionInput]}
                  value={option}
                  onChangeText={(nextValue) => {
                    const next = [...options];
                    next[index] = nextValue;
                    setOptions(next);
                  }}
                  placeholder={`Option ${index + 1}`}
                  placeholderTextColor={appColors.textSubtle}
                />
                <View style={styles.optionActionRow}>
                  <TouchableOpacity style={styles.inlineButton} onPress={() => moveOption(index, index - 1)}>
                    <Text style={styles.inlineButtonText}>↑</Text>
                  </TouchableOpacity>
                  <TouchableOpacity style={styles.inlineButton} onPress={() => moveOption(index, index + 1)}>
                    <Text style={styles.inlineButtonText}>↓</Text>
                  </TouchableOpacity>
                  <TouchableOpacity
                    style={[styles.inlineButton, options.length <= 2 && styles.inlineButtonDisabled]}
                    disabled={options.length <= 2}
                    onPress={() => {
                      setOptions(options.filter((_, optionIndex) => optionIndex !== index));
                    }}
                  >
                    <Text style={[styles.inlineButtonText, options.length <= 2 && styles.inlineButtonTextDisabled]}>x</Text>
                  </TouchableOpacity>
                </View>
              </View>
            ))}

            <TouchableOpacity style={styles.addOptionButton} onPress={() => setOptions((prev) => [...prev, ""])}>
              <Text style={styles.addOptionText}>+ Add option</Text>
            </TouchableOpacity>
          </>
        )}
        {formError && (
          <View style={[commonStyles.feedbackBanner, commonStyles.errorBanner]}>
            <Text style={commonStyles.errorBannerText}>{formError}</Text>
          </View>
        )}

        <TouchableOpacity
          style={[commonStyles.primaryButtonLarge, styles.submitButton, submitting && commonStyles.primaryButtonDisabled]}
          disabled={submitting}
          onPress={submit}
        >
          <Text style={commonStyles.primaryButtonLargeText}>{submitting ? "Creating..." : "Create Poll"}</Text>
        </TouchableOpacity>
      </KeyboardAwareScrollView>
    </KeyboardAvoidingView>
  );
}

const styles = StyleSheet.create({
  segmentedContainer: {
    borderRadius: 12,
    borderWidth: 1,
    borderColor: appColors.border,
    overflow: "hidden",
  },
  segment: {
    paddingHorizontal: 12,
    paddingVertical: 10,
    borderBottomWidth: 1,
    borderBottomColor: appColors.borderSoft,
    backgroundColor: appColors.surfaceRaised,
  },
  segmentActive: {
    backgroundColor: appColors.accentSoft,
  },
  segmentLabel: {
    color: appColors.textSubtle,
    fontWeight: "700",
    fontSize: 14,
  },
  segmentLabelActive: {
    color: appColors.text,
  },
  typeDescription: {
    marginTop: 8,
    color: appColors.textMuted,
    fontSize: 13,
  },
  submitButton: {
    marginTop: 24,
  },
  optionRow: {
    flexDirection: "row",
    alignItems: "center",
    gap: 8,
    marginBottom: 8,
  },
  optionRowDragging: {
    opacity: 0.8,
  },
  optionInput: {
    flex: 1,
  },
  hint: {
    color: appColors.textMuted,
    fontSize: 12,
    marginBottom: 8,
  },
  inlineButton: {
    minHeight: 36,
    borderRadius: 6,
    borderWidth: 1,
    borderColor: appColors.border,
    paddingHorizontal: 12,
    alignItems: "center",
    justifyContent: "center",
    backgroundColor: appColors.surfaceRaised,
  },
  optionActionRow: {
    flexDirection: "row",
    gap: 8,
    marginLeft: "auto",
  },
  inlineButtonText: {
    color: appColors.text,
    fontWeight: "600",
    fontSize: 12,
  },
  inlineButtonDisabled: {
    opacity: 0.45,
  },
  inlineButtonTextDisabled: {
    color: appColors.textMuted,
  },
  addOptionButton: {
    marginTop: 8,
    alignSelf: "flex-start",
  },
  addOptionText: {
    color: appColors.accent,
    fontWeight: "700",
  },
});
