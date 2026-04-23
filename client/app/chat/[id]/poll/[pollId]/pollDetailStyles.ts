import { StyleSheet } from "react-native";
import { appColors } from "../../../../../lib/styles";

export const pollDetailStyles = StyleSheet.create({
  question: {
    marginBottom: 4,
  },
  metaWithTop: {
    marginTop: 2,
  },
  optionRow: {
    marginBottom: 8,
    flexDirection: "row",
    justifyContent: "space-between",
    alignItems: "center",
  },
  optionRowSelected: {
    borderColor: appColors.accent,
    backgroundColor: appColors.accentSoft,
  },
  optionText: {
    color: appColors.text,
    fontWeight: "600",
  },
  multilineInput: {
    minHeight: 96,
    textAlignVertical: "top",
    marginBottom: 8,
  },
  formSwitchRow: {
    marginVertical: 8,
  },
  inlineActions: {
    marginTop: 12,
  },
  optionInputSpacing: {
    marginBottom: 8,
  },
  countRow: {
    flexDirection: "row",
    gap: 8,
    marginTop: 8,
  },
  countChip: {
    flex: 1,
    borderWidth: 1,
    borderColor: appColors.borderSoft,
    borderRadius: 10,
    paddingVertical: 10,
    paddingHorizontal: 8,
    backgroundColor: appColors.surfaceRaised,
  },
  countValue: {
    color: appColors.text,
    fontWeight: "700",
    fontSize: 16,
  },
  countLabel: {
    color: appColors.textMuted,
    fontSize: 12,
    marginTop: 2,
  },
  removeButton: {
    marginTop: 12,
    alignSelf: "flex-start",
    paddingVertical: 6,
  },
  removeButtonText: {
    color: appColors.danger,
    fontSize: 13,
    fontWeight: "600",
  },
});
