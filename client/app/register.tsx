import { useState } from "react";
import { StyleSheet, Text, TextInput, TouchableOpacity, View } from "react-native";
import { Redirect, useLocalSearchParams, useRouter } from "expo-router";

import { useAuth } from "../lib/auth";
import { appColors, commonStyles } from "../lib/styles";
import TypescriptUtils from "../lib/TypescriptUtils";
import { getApiError } from "../lib/ApiRequestError";

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
  const [emailError, setEmailError] = useState<string | null>(null);
  const [passwordError, setPasswordError] = useState<string | null>(null);
  const [formError, setFormError] = useState<string | null>(null);

  const submit = async () => {
    setEmailError(null);
    setPasswordError(null);
    setFormError(null);
    const normalizedFirstName = TypescriptUtils.parseString(firstName)?.trim() ?? "";
    const normalizedLastName = TypescriptUtils.parseString(lastName)?.trim() ?? "";
    const normalizedEmail = TypescriptUtils.parseString(email)?.trim() ?? "";

    if (
      TypescriptUtils.isNullOrWhiteSpace(normalizedFirstName) ||
      TypescriptUtils.isNullOrWhiteSpace(normalizedLastName) ||
      TypescriptUtils.isNullOrWhiteSpace(normalizedEmail) ||
      TypescriptUtils.isNullOrEmpty(password)
    ) {
      setFormError("Fill out all fields.");
      return;
    }

    if (password.length < 8) {
      setPasswordError("Password must be at least 8 characters.");
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
      const apiError = getApiError(e);
      const normalized = apiError.message.toLowerCase();
      if (apiError.field === "email" || normalized.includes("email")) {
        setEmailError(apiError.message);
      } else if (apiError.field === "password" || normalized.includes("password")) {
        setPasswordError(apiError.message);
      } else {
        setFormError(apiError.message || "Unable to register.");
      }
    } finally {
      setSubmitting(false);
    }
  };

  return (
    <View style={commonStyles.authScreen}>
      <Text style={commonStyles.authTitle}>Create Account</Text>
      <Text style={styles.subtitle}>Set up your profile to start creating chats.</Text>

      <Text style={styles.fieldLabel}>First name</Text>
      <TextInput style={[commonStyles.input, styles.inputSpacing]} placeholder="John" placeholderTextColor={appColors.textSubtle} value={firstName} onChangeText={(value) => {
        setFirstName(value);
        setFormError(null);
      }} />
      <Text style={styles.fieldLabel}>Last name</Text>
      <TextInput style={[commonStyles.input, styles.inputSpacing]} placeholder="Smith" placeholderTextColor={appColors.textSubtle} value={lastName} onChangeText={(value) => {
        setLastName(value);
        setFormError(null);
      }} />
      <Text style={styles.fieldLabel}>Email</Text>
      <TextInput
        style={[commonStyles.input, styles.inputSpacing]}
        autoCapitalize="none"
        keyboardType="email-address"
        placeholder="name@company.com"
        placeholderTextColor={appColors.textSubtle}
        value={email}
        onChangeText={(value) => {
          setEmail(value);
          setEmailError(null);
          setFormError(null);
        }}
      />
      {emailError && <Text style={commonStyles.errorText}>{emailError}</Text>}
      <Text style={styles.fieldLabel}>Password</Text>
      <TextInput
        style={[commonStyles.input, styles.inputSpacing]}
        placeholder="At least 8 characters"
        placeholderTextColor={appColors.textSubtle}
        secureTextEntry
        value={password}
        onChangeText={(value) => {
          setPassword(value);
          setPasswordError(null);
          setFormError(null);
        }}
      />
      {passwordError && <Text style={commonStyles.errorText}>{passwordError}</Text>}
      {formError && <Text style={commonStyles.errorText}>{formError}</Text>}
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
