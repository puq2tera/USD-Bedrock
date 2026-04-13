import { useState } from "react";
import { Alert, StyleSheet, Text, TextInput, TouchableOpacity, View } from "react-native";
import { Redirect, useRouter } from "expo-router";

import { useAuth } from "../lib/auth";
import { commonStyles } from "../lib/styles";
import TypescriptUtils from "../lib/TypescriptUtils";

export default function RegisterScreen() {
  const { isAuthenticated, register } = useAuth();

  if (isAuthenticated) {
    return <Redirect href="/" />;
  }

  const router = useRouter();
  const [firstName, setFirstName] = useState("");
  const [lastName, setLastName] = useState("");
  const [email, setEmail] = useState("");
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
      <Text style={commonStyles.authTitle}>Create account</Text>
      <TextInput style={[commonStyles.input, styles.inputSpacing]} placeholder="First name" value={firstName} onChangeText={setFirstName} />
      <TextInput style={[commonStyles.input, styles.inputSpacing]} placeholder="Last name" value={lastName} onChangeText={setLastName} />
      <TextInput
        style={[commonStyles.input, styles.inputSpacing]}
        autoCapitalize="none"
        keyboardType="email-address"
        placeholder="Email"
        value={email}
        onChangeText={setEmail}
      />
      <TextInput
        style={[commonStyles.input, styles.inputSpacing]}
        placeholder="Password (8+ chars)"
        secureTextEntry
        value={password}
        onChangeText={setPassword}
      />
      <TouchableOpacity style={commonStyles.primaryButton} disabled={submitting} onPress={submit}>
        <Text style={commonStyles.primaryButtonText}>{submitting ? "Creating..." : "Create account"}</Text>
      </TouchableOpacity>
    </View>
  );
}

const styles = StyleSheet.create({
  inputSpacing: { marginBottom: 12 },
});
