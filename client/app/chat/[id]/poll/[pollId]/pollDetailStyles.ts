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
});
