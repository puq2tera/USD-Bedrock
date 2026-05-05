import { useCallback, useEffect, useMemo, useState } from "react";
import { Alert, StyleSheet, Text, TextInput, TouchableOpacity, View } from "react-native";
import { Redirect, useFocusEffect, useRouter } from "expo-router";
import { KeyboardAwareScrollView } from "../components/KeyboardAwareScrollView";
import { deleteAccount, getAccount, updateAccount } from "../lib/api";
import { refreshAccountProfile, useAuth } from "../lib/auth";
import { formatDateTimeForDisplay } from "../lib/dateTime";
import { appColors, commonStyles } from "../lib/styles";

const DELETE_CONFIRMATION_PHRASE = "DELETE";

export default function AccountScreen() {
  const { isAuthenticated, user, logout } = useAuth();
  const router = useRouter();

  const [firstName, setFirstName] = useState(user?.firstName ?? "");
  const [lastName, setLastName] = useState(user?.lastName ?? "");
  const [displayName, setDisplayName] = useState(user?.displayName ?? "");
  const [email, setEmail] = useState(user?.email ?? "");
  const [createdAt, setCreatedAt] = useState("");
  const [deletePhrase, setDeletePhrase] = useState("");
  const [saving, setSaving] = useState(false);
  const [showSavedBanner, setShowSavedBanner] = useState(false);

  useEffect(() => {
    setFirstName(user?.firstName ?? "");
    setLastName(user?.lastName ?? "");
    setDisplayName(user?.displayName ?? "");
    setEmail(user?.email ?? "");
  }, [user?.displayName, user?.email, user?.firstName, user?.lastName]);

  useFocusEffect(
    useCallback(() => {
      if (!isAuthenticated) {
        return;
      }

      void (async () => {
        try {
          const account = await getAccount();
          setEmail(account.email);
          setCreatedAt(account.createdAt);
        } catch {
          // Keep last known account metadata if refresh fails.
        }
      })();
    }, [isAuthenticated])
  );

  useEffect(() => {
    if (!showSavedBanner) {
      return;
    }

    const timeout = setTimeout(() => setShowSavedBanner(false), 2500);
    return () => clearTimeout(timeout);
  }, [showSavedBanner]);

  const canPersist = useMemo(
    () => [firstName, lastName, displayName, email].some((value) => value.trim().length > 0),
    [displayName, email, firstName, lastName]
  );
  const memberSinceLabel = useMemo(() => formatDateTimeForDisplay(createdAt), [createdAt]);

  if (!isAuthenticated) {
    return <Redirect href="/login" />;
  }

  const persistProfile = async () => {
    if (!canPersist || saving) {
      return;
    }

    setSaving(true);
    try {
      await updateAccount({
        firstName: firstName.trim() || undefined,
        lastName: lastName.trim() || undefined,
        displayName: displayName.trim() || undefined,
        email: email.trim() || undefined,
      });
      await refreshAccountProfile();
      setShowSavedBanner(true);
    } catch (e: any) {
      Alert.alert("Profile update failed", e?.message || "Please retry.");
    } finally {
      setSaving(false);
    }
  };

  const performDelete = async () => {
    if (deletePhrase.trim() !== DELETE_CONFIRMATION_PHRASE) {
      Alert.alert("Confirmation mismatch", `Type ${DELETE_CONFIRMATION_PHRASE} to delete your account.`);
      return;
    }

    Alert.alert("Delete Account", "This permanently deletes your account and session data.", [
      { text: "Cancel", style: "cancel" },
      {
        text: "Delete Account",
        style: "destructive",
        onPress: async () => {
          setSaving(true);
          try {
            await deleteAccount();
            await logout();
            router.replace("/login");
          } catch (e: any) {
            Alert.alert("Delete failed", e?.message || "Please retry.");
          } finally {
            setSaving(false);
          }
        },
      },
    ]);
  };

  return (
    <KeyboardAwareScrollView style={commonStyles.screen} contentContainerStyle={commonStyles.screenContent}>
      <View style={commonStyles.sectionCard}>
        <Text style={commonStyles.pageTitle}>{displayName || user?.displayName || "Account"}</Text>
        <Text style={[commonStyles.metaText, styles.metaWithTop]}>{email || user?.email || ""}</Text>
        <Text style={[commonStyles.metaText, styles.metaWithTop]}>
          Member since {memberSinceLabel}
        </Text>
      </View>

      <View style={commonStyles.sectionCard}>
        <View style={styles.sectionHeader}>
          <Text style={commonStyles.sectionTitle}>Profile & Settings</Text>
          {saving && <Text style={commonStyles.metaText}>Saving...</Text>}
        </View>
        {showSavedBanner && (
          <View style={[commonStyles.feedbackBanner, commonStyles.successBanner]}>
            <Text style={commonStyles.successBannerText}>Profile changes saved.</Text>
          </View>
        )}

        <Text style={commonStyles.sectionLabel}>First name</Text>
        <TextInput style={commonStyles.input} value={firstName} onChangeText={setFirstName} />

        <Text style={commonStyles.sectionLabel}>Last name</Text>
        <TextInput style={commonStyles.input} value={lastName} onChangeText={setLastName} />

        <Text style={commonStyles.sectionLabel}>Display name</Text>
        <TextInput style={commonStyles.input} value={displayName} onChangeText={setDisplayName} />

        <Text style={commonStyles.sectionLabel}>Email</Text>
        <TextInput
          style={commonStyles.input}
          value={email}
          onChangeText={setEmail}
          autoCapitalize="none"
          keyboardType="email-address"
        />

        <TouchableOpacity
          style={[commonStyles.primaryButton, styles.saveButton, saving && commonStyles.primaryButtonDisabled]}
          disabled={saving || !canPersist}
          onPress={() => void persistProfile()}
        >
          <Text style={commonStyles.primaryButtonText}>Save Profile Changes</Text>
        </TouchableOpacity>
      </View>

      <TouchableOpacity
        style={[commonStyles.primaryButtonLarge, styles.signOutButton, saving && commonStyles.primaryButtonDisabled]}
        disabled={saving}
        onPress={async () => {
          await logout();
          router.replace("/login");
        }}
      >
        <Text style={commonStyles.primaryButtonLargeText}>Sign Out</Text>
      </TouchableOpacity>

      <View style={[commonStyles.sectionCard, styles.dangerCard]}>
        <Text style={styles.dangerTitle}>Delete Account</Text>
        <Text style={[commonStyles.metaText, styles.metaWithTop]}>
          Type {DELETE_CONFIRMATION_PHRASE} to permanently remove this account.
        </Text>
        <TextInput
          style={[commonStyles.input, styles.deleteInput]}
          value={deletePhrase}
          onChangeText={setDeletePhrase}
          autoCapitalize="characters"
        />
        <TouchableOpacity style={[commonStyles.primaryButton, styles.deleteButton]} onPress={performDelete}>
          <Text style={commonStyles.primaryButtonText}>Delete Account</Text>
        </TouchableOpacity>
      </View>
    </KeyboardAwareScrollView>
  );
}

const styles = StyleSheet.create({
  metaWithTop: {
    marginTop: 6,
  },
  sectionHeader: {
    flexDirection: "row",
    alignItems: "center",
    justifyContent: "space-between",
  },
  signOutButton: {
    marginTop: 6,
  },
  saveButton: {
    marginTop: 16,
  },
  dangerCard: {
    marginTop: 14,
    borderWidth: 1,
    borderColor: appColors.danger,
  },
  dangerTitle: {
    color: appColors.danger,
    fontSize: 16,
    fontWeight: "700",
  },
  deleteInput: {
    marginTop: 10,
  },
  deleteButton: {
    marginTop: 10,
    backgroundColor: appColors.danger,
  },
});
