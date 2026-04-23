import { StyleSheet } from "react-native";
import { appColors } from "../../../lib/styles";

export const chatDetailStyles = StyleSheet.create({
  metaWithTop: {
    marginTop: 4,
  },
  messageRow: {
    marginBottom: 10,
  },
  messageAuthor: {
    marginBottom: 4,
  },
  messageBody: {
    color: appColors.text,
    fontSize: 14,
  },
  messageEditInput: {
    minHeight: 68,
    textAlignVertical: "top",
  },
  messageComposer: {
    marginTop: 10,
    minHeight: 76,
    textAlignVertical: "top",
    marginBottom: 8,
  },
  pollRow: {
    marginBottom: 8,
  },
  pollQuestion: {
    marginBottom: 2,
  },
  inlineComposer: {
    flexDirection: "row",
    gap: 8,
    alignItems: "center",
    marginBottom: 8,
  },
  inlineInput: {
    flex: 1,
  },
  memberRow: {
    marginBottom: 8,
    flexDirection: "row",
    justifyContent: "space-between",
    alignItems: "center",
  },
  memberIdentityRow: {
    flexDirection: "row",
    alignItems: "center",
    gap: 4,
    flexShrink: 1,
    paddingRight: 10,
  },
  memberRoleText: {
    color: appColors.textMuted,
    fontStyle: "italic",
  },
  memberName: {},
});
