import { useState } from "react";
import { Alert, StyleSheet, Text, TextInput, TouchableOpacity, View } from "react-native";
import { Redirect, useRouter } from "expo-router";

import { useAuth } from "../lib/auth";
import { appColors, commonStyles } from "../lib/styles";
import TypescriptUtils from "../lib/TypescriptUtils";

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

  const submit = async () => {
    const normalizedEmail = TypescriptUtils.parseString(email)?.trim() ?? "";
    if (TypescriptUtils.isNullOrWhiteSpace(normalizedEmail)) {
      Alert.alert("Missing Email", "Enter your email.");
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
        Alert.alert("Unable To Continue", e?.message || "Please retry.");
      } finally {
        setSubmitting(false);
      }
      return;
    }

    if (TypescriptUtils.isNullOrEmpty(password)) {
      Alert.alert("Missing Password", "Enter your password.");
      return;
    }

    try {
      setSubmitting(true);
      await login(normalizedEmail, password);
      router.replace("/");
    } catch (e: any) {
      Alert.alert("Login Failed", e?.message || "Unable to login.");
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
          if (needsPassword && nextEmail.trim() !== checkedEmail) {
            setNeedsPassword(false);
            setPassword("");
          }
        }}
      />
      {needsPassword && (
        <>
          <Text style={styles.fieldLabel}>Password</Text>
          <TextInput
            style={[commonStyles.input, styles.inputSpacing]}
            placeholder="Enter your password"
            placeholderTextColor={appColors.textSubtle}
            secureTextEntry
            value={password}
            onChangeText={setPassword}
          />
        </>
      )}
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
