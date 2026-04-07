import { StyleSheet } from "react-native";

export const appColors = {
  background: "#f5f5f5",
  surface: "#fff",
  text: "#1a1a1a",
  textMuted: "#888",
  textSubtle: "#555",
  border: "#ddd",
  borderSoft: "#e0e0e0",
  accent: "#0D7E3F",
  accentSoft: "rgba(13, 126, 63, 0.12)",
  danger: "#d32f2f",
  neutralButton: "#eee",
  neutralButtonText: "#999",
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
    backgroundColor: appColors.surface,
    borderRadius: appRadii.input,
    borderWidth: 1,
    borderColor: appColors.border,
    paddingHorizontal: appSpacing.inputInset,
    paddingVertical: 12,
    fontSize: 16,
  },
  card: {
    backgroundColor: appColors.surface,
    borderRadius: appRadii.card,
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
  headerStyle: { backgroundColor: appColors.accent },
  headerTintColor: appColors.surface,
  headerTitleStyle: { fontWeight: "bold" as const },
};
