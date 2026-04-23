import { useState } from "react";
import { Alert, StyleSheet, Text, TextInput, TouchableOpacity, View } from "react-native";
import { Link, Redirect, useRouter } from "expo-router";

import { useAuth } from "../lib/auth";
import { appColors, commonStyles } from "../lib/styles";
import TypescriptUtils from "../lib/TypescriptUtils";

export default function LoginScreen() {
  const { isAuthenticated, login } = useAuth();

  if (isAuthenticated) {
    return <Redirect href="/" />;
  }

  const router = useRouter();
  const [email, setEmail] = useState("");
  const [password, setPassword] = useState("");
  const [submitting, setSubmitting] = useState(false);

  const submit = async () => {
    const normalizedEmail = TypescriptUtils.parseString(email)?.trim() ?? "";
    if (TypescriptUtils.isNullOrWhiteSpace(normalizedEmail) || TypescriptUtils.isNullOrEmpty(password)) {
      Alert.alert("Missing fields", "Enter your email and password.");
      return;
    }

    try {
      setSubmitting(true);
      await login(normalizedEmail, password);
      router.replace("/");
    } catch (e: any) {
      Alert.alert("Login failed", e?.message || "Unable to login.");
    } finally {
      setSubmitting(false);
    }
  };

  return (
    <View style={commonStyles.authScreen}>
      <Text style={commonStyles.authTitle}>Welcome back</Text>
      <Text style={styles.subtitle}>Continue to your chats and polls.</Text>

      <Text style={styles.fieldLabel}>Email</Text>
      <TextInput
        style={[commonStyles.input, styles.inputSpacing]}
        autoCapitalize="none"
        keyboardType="email-address"
        placeholder="name@company.com"
        value={email}
        onChangeText={setEmail}
      />
      <Text style={styles.fieldLabel}>Password</Text>
      <TextInput
        style={[commonStyles.input, styles.inputSpacing]}
        placeholder="Enter your password"
        secureTextEntry
        value={password}
        onChangeText={setPassword}
      />
      <TouchableOpacity style={commonStyles.primaryButton} disabled={submitting} onPress={submit}>
        <Text style={commonStyles.primaryButtonText}>{submitting ? "Logging in..." : "Login"}</Text>
      </TouchableOpacity>
      <Link href="/register" style={[commonStyles.textLink, styles.link]}>
        Create an account
      </Link>
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
  link: { marginTop: 16, textAlign: "center" },
});
