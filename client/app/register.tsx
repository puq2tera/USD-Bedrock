import { useState } from "react";
import { Alert, StyleSheet, Text, TextInput, TouchableOpacity, View } from "react-native";
import { Redirect, useLocalSearchParams, useRouter } from "expo-router";

import { useAuth } from "../lib/auth";
import { appColors, commonStyles } from "../lib/styles";
import TypescriptUtils from "../lib/TypescriptUtils";

export default function RegisterScreen() {
  const { isAuthenticated, register } = useAuth();

  if (isAuthenticated) {
    return <Redirect href="/" />;
  }

  const router = useRouter();
  const { email: prefilledEmail } = useLocalSearchParams<{ email?: string }>();
  const [firstName, setFirstName] = useState("");
  const [lastName, setLastName] = useState("");
  const [email, setEmail] = useState(TypescriptUtils.parseString(prefilledEmail) ?? "");
  const [password, setPassword] = useState("");
  const [submitting, setSubmitting] = useState(false);

  const submit = async () => {
    const normalizedFirstName = TypescriptUtils.parseString(firstName)?.trim() ?? "";
    const normalizedLastName = TypescriptUtils.parseString(lastName)?.trim() ?? "";
    const normalizedEmail = TypescriptUtils.parseString(email)?.trim() ?? "";

    if (
      TypescriptUtils.isNullOrWhiteSpace(normalizedFirstName) ||
      TypescriptUtils.isNullOrWhiteSpace(normalizedLastName) ||
      TypescriptUtils.isNullOrWhiteSpace(normalizedEmail) ||
      TypescriptUtils.isNullOrEmpty(password)
    ) {
      Alert.alert("Missing fields", "Fill out all fields.");
      return;
    }

    if (password.length < 8) {
      Alert.alert("Weak password", "Password must be at least 8 characters.");
      return;
    }

    try {
      setSubmitting(true);
      await register({
        firstName: normalizedFirstName,
        lastName: normalizedLastName,
        email: normalizedEmail,
        password,
      });
      router.replace("/");
    } catch (e: any) {
      Alert.alert("Registration failed", e?.message || "Unable to register.");
    } finally {
      setSubmitting(false);
    }
  };

  return (
    <View style={commonStyles.authScreen}>
      <Text style={commonStyles.authTitle}>Create Account</Text>
      <Text style={styles.subtitle}>Set up your profile to start creating chats.</Text>

      <Text style={styles.fieldLabel}>First name</Text>
      <TextInput style={[commonStyles.input, styles.inputSpacing]} placeholder="Ava" placeholderTextColor={appColors.textSubtle} value={firstName} onChangeText={setFirstName} />
      <Text style={styles.fieldLabel}>Last name</Text>
      <TextInput style={[commonStyles.input, styles.inputSpacing]} placeholder="Nguyen" placeholderTextColor={appColors.textSubtle} value={lastName} onChangeText={setLastName} />
      <Text style={styles.fieldLabel}>Email</Text>
      <TextInput
        style={[commonStyles.input, styles.inputSpacing]}
        autoCapitalize="none"
        keyboardType="email-address"
        placeholder="name@company.com"
        placeholderTextColor={appColors.textSubtle}
        value={email}
        onChangeText={setEmail}
      />
      <Text style={styles.fieldLabel}>Password</Text>
      <TextInput
        style={[commonStyles.input, styles.inputSpacing]}
        placeholder="At least 8 characters"
        placeholderTextColor={appColors.textSubtle}
        secureTextEntry
        value={password}
        onChangeText={setPassword}
      />
      <TouchableOpacity style={commonStyles.primaryButton} disabled={submitting} onPress={submit}>
        <Text style={commonStyles.primaryButtonText}>{submitting ? "Creating..." : "Create Account"}</Text>
      </TouchableOpacity>
    </View>
  );
}

const styles = StyleSheet.create({
  subtitle: {
    color: appColors.textMuted,
    fontSize: 12,
    marginBottom: 16,
  },
  fieldLabel: {
    color: "#c7d6ee",
    fontSize: 14,
    fontWeight: "600",
    marginBottom: 6,
  },
  inputSpacing: { marginBottom: 12 },
});
