import { StyleSheet } from "react-native";
import { appColors } from "../../../../../lib/styles";

export const pollDetailStyles = StyleSheet.create({
  question: {
    marginBottom: 4,
  },
  metaWithTop: {
    marginTop: 2,
  },
  summaryMetaRow: {
    marginTop: 8,
    flexDirection: "row",
    alignItems: "center",
    gap: 8,
  },
  summaryStatsRow: {
    marginTop: 10,
    flexDirection: "row",
    alignItems: "center",
    flexWrap: "wrap",
    gap: 6,
  },
  summaryStatValue: {
    color: appColors.text,
    fontSize: 16,
    fontWeight: "700",
  },
  summaryStatLabel: {
    color: appColors.textMuted,
    fontSize: 13,
  },
  summaryStatDivider: {
    color: appColors.textMuted,
    marginHorizontal: 2,
  },
  stateChip: {
    borderRadius: 999,
    borderWidth: 1,
    paddingHorizontal: 10,
    paddingVertical: 4,
  },
  stateChipOpen: {
    borderColor: appColors.accent,
    backgroundColor: appColors.accentSoft,
  },
  stateChipClosed: {
    borderColor: appColors.border,
    backgroundColor: appColors.surfaceRaised,
  },
  stateChipText: {
    color: appColors.text,
    fontSize: 12,
    fontWeight: "700",
  },
  optionRow: {
    marginBottom: 8,
    flexDirection: "row",
    justifyContent: "space-between",
    alignItems: "center",
  },
  optionLeading: {
    marginRight: 10,
  },
  optionRowSelected: {
    borderColor: appColors.accent,
    backgroundColor: appColors.accentSoft,
  },
  optionText: {
    color: appColors.text,
    fontWeight: "600",
    flex: 1,
  },
  selectionHint: {
    marginBottom: 8,
  },
  freeTextIntro: {
    marginTop: 8,
    marginBottom: 10,
  },
  singleChoiceRing: {
    width: 18,
    height: 18,
    borderRadius: 9,
    borderWidth: 2,
    borderColor: appColors.textMuted,
    alignItems: "center",
    justifyContent: "center",
  },
  singleChoiceRingSelected: {
    borderColor: appColors.accent,
  },
  singleChoiceDot: {
    width: 8,
    height: 8,
    borderRadius: 4,
    backgroundColor: appColors.accent,
  },
  multiChoiceBox: {
    width: 18,
    height: 18,
    borderRadius: 5,
    borderWidth: 2,
    borderColor: appColors.textMuted,
    alignItems: "center",
    justifyContent: "center",
  },
  multiChoiceBoxSelected: {
    borderColor: appColors.accent,
    backgroundColor: appColors.accent,
  },
  multiChoiceCheck: {
    color: appColors.background,
    fontSize: 11,
    fontWeight: "800",
    lineHeight: 12,
  },
  rankChoiceBadge: {
    minWidth: 22,
    height: 18,
    borderRadius: 9,
    borderWidth: 1,
    borderColor: appColors.textMuted,
    alignItems: "center",
    justifyContent: "center",
    paddingHorizontal: 5,
  },
  rankChoiceBadgeSelected: {
    borderColor: appColors.accent,
    backgroundColor: appColors.accent,
  },
  rankChoiceText: {
    color: appColors.textMuted,
    fontSize: 11,
    fontWeight: "700",
    lineHeight: 12,
  },
  rankChoiceTextSelected: {
    color: appColors.background,
  },
  multilineInput: {
    minHeight: 96,
    textAlignVertical: "top",
    marginBottom: 10,
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
    flexDirection: "column",
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
