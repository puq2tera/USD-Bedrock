import { useState } from "react";
import { StyleSheet, Text, TextInput, TouchableOpacity, View } from "react-native";
import { Redirect, useRouter } from "expo-router";

import { useAuth } from "../lib/auth";
import { appColors, commonStyles } from "../lib/styles";
import TypescriptUtils from "../lib/TypescriptUtils";
import { getApiError } from "../lib/ApiRequestError";

export default function LoginScreen() {
  const { isAuthenticated, checkEmailExists, login } = useAuth();

  if (isAuthenticated) {
    return <Redirect href="/" />;
  }

  const router = useRouter();
  const [email, setEmail] = useState("");
  const [password, setPassword] = useState("");
  const [submitting, setSubmitting] = useState(false);
  const [needsPassword, setNeedsPassword] = useState(false);
  const [checkedEmail, setCheckedEmail] = useState("");
  const [emailError, setEmailError] = useState<string | null>(null);
  const [passwordError, setPasswordError] = useState<string | null>(null);
  const [formError, setFormError] = useState<string | null>(null);

  const submit = async () => {
    setEmailError(null);
    setPasswordError(null);
    setFormError(null);
    const normalizedEmail = TypescriptUtils.parseString(email)?.trim() ?? "";
    if (TypescriptUtils.isNullOrWhiteSpace(normalizedEmail)) {
      setEmailError("Enter your email.");
      return;
    }

    if (!needsPassword || checkedEmail !== normalizedEmail) {
      try {
        setSubmitting(true);
        const exists = await checkEmailExists(normalizedEmail);
        if (!exists) {
          router.push(`/register?email=${encodeURIComponent(normalizedEmail)}`);
          return;
        }

        setCheckedEmail(normalizedEmail);
        setNeedsPassword(true);
      } catch (e: any) {
        const apiError = getApiError(e);
        setFormError(apiError.message || "Unable to continue. Please retry.");
      } finally {
        setSubmitting(false);
      }
      return;
    }

    if (TypescriptUtils.isNullOrEmpty(password)) {
      setPasswordError("Enter your password.");
      return;
    }

    try {
      setSubmitting(true);
      await login(normalizedEmail, password);
      router.replace("/");
    } catch (e: any) {
      const apiError = getApiError(e);
      const normalized = apiError.message.toLowerCase();
      if (apiError.status === 401 || normalized.includes("unauthorized") || normalized.includes("invalid email or password")) {
        setPasswordError("Invalid email or password.");
      } else {
        setFormError(apiError.message || "Unable to login.");
      }
    } finally {
      setSubmitting(false);
    }
  };

  return (
    <View style={commonStyles.authScreen}>
      <Text style={commonStyles.authTitle}>Welcome</Text>

      <Text style={styles.fieldLabel}>Email</Text>
      <TextInput
        style={[commonStyles.input, styles.inputSpacing]}
        autoCapitalize="none"
        keyboardType="email-address"
        placeholder="name@company.com"
        placeholderTextColor={appColors.textSubtle}
        value={email}
        onChangeText={(nextEmail) => {
          setEmail(nextEmail);
          setEmailError(null);
          setFormError(null);
          if (needsPassword && nextEmail.trim() !== checkedEmail) {
            setNeedsPassword(false);
            setPassword("");
            setPasswordError(null);
          }
        }}
      />
      {emailError && <Text style={commonStyles.errorText}>{emailError}</Text>}
      {needsPassword && (
        <>
          <Text style={styles.fieldLabel}>Password</Text>
          <TextInput
            style={[commonStyles.input, styles.inputSpacing]}
            placeholder="Enter your password"
            placeholderTextColor={appColors.textSubtle}
            secureTextEntry
            value={password}
            onChangeText={(nextPassword) => {
              setPassword(nextPassword);
              setPasswordError(null);
              setFormError(null);
            }}
          />
          {passwordError && <Text style={commonStyles.errorText}>{passwordError}</Text>}
        </>
      )}
      {formError && <Text style={commonStyles.errorText}>{formError}</Text>}
      <TouchableOpacity style={commonStyles.primaryButton} disabled={submitting} onPress={submit}>
        <Text style={commonStyles.primaryButtonText}>
          {submitting ? "Working..." : (needsPassword ? "Login" : "Continue")}
        </Text>
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
