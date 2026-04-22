import { useMemo, useState } from "react";
import {
  Alert,
  KeyboardAvoidingView,
  Platform,
  ScrollView,
  StyleSheet,
  Text,
  TextInput,
  TouchableOpacity,
  View,
} from "react-native";
import { Redirect, useRouter } from "expo-router";
import { deleteAccount, updateAccount } from "../lib/api";
import { refreshAccountProfile, useAuth } from "../lib/auth";
import { appColors, commonStyles } from "../lib/styles";

const DELETE_CONFIRMATION_PHRASE = "DELETE";

export default function SettingsScreen() {
  const { isAuthenticated, user, logout } = useAuth();
  const router = useRouter();

  const [firstName, setFirstName] = useState(user?.firstName ?? "");
  const [lastName, setLastName] = useState(user?.lastName ?? "");
  const [displayName, setDisplayName] = useState(user?.displayName ?? "");
  const [email, setEmail] = useState(user?.email ?? "");
  const [deletePhrase, setDeletePhrase] = useState("");
  const [busy, setBusy] = useState(false);

  const canSave = useMemo(
    () => [firstName, lastName, displayName, email].some((value) => value.trim().length > 0),
    [firstName, lastName, displayName, email]
  );

  if (!isAuthenticated) {
    return <Redirect href="/login" />;
  }

  const saveProfile = async () => {
    if (!canSave) {
      Alert.alert("Missing fields", "Provide at least one value to update.");
      return;
    }

    setBusy(true);
    try {
      await updateAccount({
        firstName: firstName.trim() || undefined,
        lastName: lastName.trim() || undefined,
        displayName: displayName.trim() || undefined,
        email: email.trim() || undefined,
      });
      await refreshAccountProfile();
      Alert.alert("Saved", "Profile updated.");
    } catch (e: any) {
      Alert.alert("Profile update failed", e?.message || "Please retry.");
    } finally {
      setBusy(false);
    }
  };

  const performDelete = async () => {
    if (deletePhrase.trim() !== DELETE_CONFIRMATION_PHRASE) {
      Alert.alert("Confirmation mismatch", `Type ${DELETE_CONFIRMATION_PHRASE} to delete your account.`);
      return;
    }

    Alert.alert("Delete account", "This permanently deletes your account and session data.", [
      { text: "Cancel", style: "cancel" },
      {
        text: "Delete account",
        style: "destructive",
        onPress: async () => {
          setBusy(true);
          try {
            await deleteAccount();
            await logout();
            router.replace("/login");
          } catch (e: any) {
            Alert.alert("Delete failed", e?.message || "Please retry.");
          } finally {
            setBusy(false);
          }
        },
      },
    ]);
  };

  return (
    <KeyboardAvoidingView style={commonStyles.screen} behavior={Platform.OS === "ios" ? "padding" : undefined}>
      <ScrollView contentContainerStyle={commonStyles.screenContent}>
        <Text style={commonStyles.sectionLabel}>First name</Text>
        <TextInput style={commonStyles.input} value={firstName} onChangeText={setFirstName} />

        <Text style={commonStyles.sectionLabel}>Last name</Text>
        <TextInput style={commonStyles.input} value={lastName} onChangeText={setLastName} />

        <Text style={commonStyles.sectionLabel}>Display name</Text>
        <TextInput style={commonStyles.input} value={displayName} onChangeText={setDisplayName} />

        <Text style={commonStyles.sectionLabel}>Email</Text>
        <TextInput style={commonStyles.input} value={email} onChangeText={setEmail} autoCapitalize="none" keyboardType="email-address" />

        <TouchableOpacity style={[commonStyles.primaryButtonLarge, styles.saveButton, busy && commonStyles.primaryButtonDisabled]} disabled={busy} onPress={saveProfile}>
          <Text style={commonStyles.primaryButtonLargeText}>{busy ? "Saving..." : "Save profile"}</Text>
        </TouchableOpacity>

        <View style={[commonStyles.card, styles.dangerCard]}>
          <Text style={styles.dangerTitle}>Delete account</Text>
          <Text style={[commonStyles.metaText, styles.dangerBody]}>Type {DELETE_CONFIRMATION_PHRASE} and confirm to permanently delete your account.</Text>
          <TextInput style={[commonStyles.input, styles.deletePhraseInput]} value={deletePhrase} onChangeText={setDeletePhrase} autoCapitalize="characters" />
          <TouchableOpacity style={[commonStyles.primaryButton, styles.deleteButton]} onPress={performDelete}>
            <Text style={commonStyles.primaryButtonText}>Delete account</Text>
          </TouchableOpacity>
        </View>
      </ScrollView>
    </KeyboardAvoidingView>
  );
}

const styles = StyleSheet.create({
  dangerCard: {
    marginTop: 24,
    borderWidth: 1,
    borderColor: appColors.danger,
    padding: 14,
  },
  saveButton: {
    marginTop: 20,
  },
  dangerTitle: {
    color: appColors.danger,
    fontWeight: "700",
    fontSize: 16,
  },
  dangerBody: {
    marginTop: 8,
  },
  deletePhraseInput: {
    marginTop: 10,
  },
  deleteButton: {
    marginTop: 10,
    backgroundColor: appColors.danger,
  },
});
