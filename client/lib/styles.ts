import { StyleSheet } from "react-native";

export const appColors = {
  background: "#071426",
  surface: "#101f35",
  surfaceRaised: "#162843",
  text: "#e6edf7",
  textMuted: "#93a4bf",
  textSubtle: "#b8c6dd",
  border: "#294366",
  borderSoft: "#1f3552",
  accent: "#1f8fff",
  accentSoft: "rgba(31, 143, 255, 0.16)",
  danger: "#ff4a57",
  neutralButton: "#1a2f4a",
  neutralButtonText: "#9eb2d1",
} as const;

export const appSpacing = {
  screenPadding: 16,
  formPadding: 20,
  inputInset: 14,
  buttonVertical: 14,
  buttonLargeVertical: 16,
  sectionGap: 16,
  itemGap: 12,
} as const;

export const appRadii = {
  input: 10,
  card: 12,
  button: 10,
  buttonSmall: 8,
  pill: 18,
  fab: 28,
} as const;

// Shared primitives belong here so screen files only keep layout and visuals
// that are unique to that screen's behavior.
export const commonStyles = StyleSheet.create({
  screen: {
    flex: 1,
    backgroundColor: appColors.background,
  },
  centeredScreen: {
    flex: 1,
    justifyContent: "center",
    alignItems: "center",
    backgroundColor: appColors.background,
  },
  screenContent: {
    padding: appSpacing.screenPadding,
    paddingBottom: 40,
  },
  authScreen: {
    flex: 1,
    justifyContent: "center",
    padding: appSpacing.formPadding,
    backgroundColor: appColors.background,
  },
  authTitle: {
    fontSize: 28,
    fontWeight: "700",
    marginBottom: 20,
    color: appColors.text,
  },
  sectionLabel: {
    fontSize: 15,
    fontWeight: "600",
    color: appColors.textSubtle,
    marginBottom: 6,
    marginTop: appSpacing.sectionGap,
  },
  input: {
    backgroundColor: appColors.surfaceRaised,
    borderRadius: appRadii.input,
    borderWidth: 1,
    borderColor: appColors.border,
    paddingHorizontal: appSpacing.inputInset,
    paddingVertical: 12,
    fontSize: 16,
    color: appColors.text,
  },
  card: {
    backgroundColor: appColors.surface,
    borderRadius: appRadii.card,
  },
  sectionCard: {
    backgroundColor: appColors.surface,
    borderRadius: appRadii.card,
    borderWidth: 1,
    borderColor: appColors.borderSoft,
    padding: 14,
    marginBottom: 14,
  },
  sectionHeaderRow: {
    flexDirection: "row",
    alignItems: "center",
    justifyContent: "space-between",
    marginBottom: 10,
  },
  sectionTitle: {
    fontSize: 17,
    fontWeight: "700",
    color: appColors.text,
  },
  pageTitle: {
    fontSize: 20,
    fontWeight: "700",
    color: appColors.text,
  },
  metaText: {
    color: appColors.textMuted,
    fontSize: 12,
  },
  inlineActionsRow: {
    flexDirection: "row",
    alignItems: "center",
    gap: 8,
    marginTop: 8,
  },
  ghostButton: {
    borderWidth: 1,
    borderColor: appColors.border,
    borderRadius: appRadii.buttonSmall,
    paddingHorizontal: 10,
    paddingVertical: 7,
    backgroundColor: appColors.surfaceRaised,
  },
  ghostButtonText: {
    color: appColors.text,
    fontWeight: "600",
  },
  miniPrimaryButton: {
    backgroundColor: appColors.accent,
    borderRadius: appRadii.buttonSmall,
    paddingHorizontal: 10,
    paddingVertical: 8,
  },
  miniPrimaryButtonText: {
    color: appColors.surface,
    fontWeight: "700",
  },
  miniDangerButton: {
    backgroundColor: appColors.danger,
    borderRadius: appRadii.buttonSmall,
    paddingHorizontal: 10,
    paddingVertical: 8,
  },
  miniDangerButtonText: {
    color: appColors.surface,
    fontWeight: "700",
  },
  dangerBlockButton: {
    marginTop: 10,
    borderRadius: appRadii.button,
    paddingVertical: 12,
    alignItems: "center",
    backgroundColor: appColors.danger,
  },
  dangerBlockButtonText: {
    color: appColors.surface,
    fontWeight: "700",
  },
  outlinedRow: {
    borderWidth: 1,
    borderColor: appColors.borderSoft,
    borderRadius: appRadii.input,
    padding: 10,
    backgroundColor: appColors.surfaceRaised,
  },
  emphasizedRowLabel: {
    fontWeight: "700",
    color: appColors.text,
  },
  listTitle: {
    marginTop: 10,
    fontWeight: "700",
    color: appColors.text,
    fontSize: 13,
  },
  formSwitchRow: {
    flexDirection: "row",
    justifyContent: "space-between",
    alignItems: "center",
    marginVertical: 8,
  },
  formSwitchLabel: {
    color: appColors.text,
    fontWeight: "600",
    fontSize: 15,
  },
  circularIconButton: {
    width: 34,
    height: 34,
    borderRadius: 17,
    alignItems: "center",
    justifyContent: "center",
    borderWidth: 1,
    borderColor: appColors.border,
    backgroundColor: appColors.surfaceRaised,
  },
  circularIconButtonText: {
    color: appColors.text,
    fontWeight: "700",
    fontSize: 16,
  },
  primaryButton: {
    backgroundColor: appColors.accent,
    borderRadius: appRadii.button,
    paddingVertical: appSpacing.buttonVertical,
    alignItems: "center",
  },
  primaryButtonLarge: {
    backgroundColor: appColors.accent,
    borderRadius: appRadii.card,
    paddingVertical: appSpacing.buttonLargeVertical,
    alignItems: "center",
  },
  primaryButtonDisabled: {
    opacity: 0.6,
  },
  primaryButtonText: {
    color: appColors.surface,
    fontWeight: "700",
    fontSize: 16,
  },
  primaryButtonLargeText: {
    color: appColors.surface,
    fontSize: 17,
    fontWeight: "bold",
  },
  textLink: {
    color: appColors.accent,
    fontWeight: "600",
  },
  errorText: {
    fontSize: 16,
    color: appColors.danger,
    marginBottom: 12,
  },
  retryButton: {
    backgroundColor: appColors.accent,
    paddingHorizontal: 24,
    paddingVertical: 10,
    borderRadius: appRadii.buttonSmall,
  },
  retryButtonText: {
    color: appColors.surface,
    fontWeight: "600",
  },
  floatingActionButton: {
    position: "absolute",
    bottom: 32,
    right: 24,
    width: 56,
    height: 56,
    borderRadius: appRadii.fab,
    backgroundColor: appColors.accent,
    justifyContent: "center",
    alignItems: "center",
    shadowColor: "#000",
    shadowOffset: { width: 0, height: 2 },
    shadowOpacity: 0.25,
    shadowRadius: 4,
    elevation: 5,
  },
  floatingActionButtonText: {
    color: appColors.surface,
    fontSize: 28,
    lineHeight: 30,
  },
});

export const appStackScreenOptions = {
  headerStyle: { backgroundColor: appColors.background },
  headerTintColor: appColors.text,
  headerBackTitleVisible: false,
  headerTitleStyle: { fontWeight: "bold" as const },
};
