import { useCallback, useEffect, useState } from "react";
import { Alert, RefreshControl, StyleSheet, Text, TextInput, TouchableOpacity, View } from "react-native";
import { Redirect, useFocusEffect, useLocalSearchParams, useRouter } from "expo-router";
import { KeyboardAwareScrollView } from "../../../components/KeyboardAwareScrollView";
import TypescriptUtils from "../../../lib/TypescriptUtils";
import {
  addChatMember,
  canManageChat,
  ChatDetail,
  ChatMember,
  deleteChat,
  editChat,
  getChat,
  getIdentityLabel,
  listChatMembers,
  lookupUserByEmail,
  removeChatMember,
} from "../../../lib/api";
import { useAuth } from "../../../lib/auth";
import { appColors, commonStyles } from "../../../lib/styles";
import { getApiError } from "../../../lib/ApiRequestError";

export default function ChatSettingsScreen() {
  const { isAuthenticated, user } = useAuth();
  const router = useRouter();
  const { id } = useLocalSearchParams<{ id: string }>();
  const chatID = TypescriptUtils.parseString(id) ?? "";

  const [chat, setChat] = useState<ChatDetail | null>(null);
  const [members, setMembers] = useState<ChatMember[]>([]);
  const [chatTitle, setChatTitle] = useState("");
  const [memberEmailDraft, setMemberEmailDraft] = useState("");
  const [memberError, setMemberError] = useState<string | null>(null);
  const [showSavedBanner, setShowSavedBanner] = useState(false);
  const [busy, setBusy] = useState(false);
  const [refreshing, setRefreshing] = useState(false);
  const [error, setError] = useState<string | null>(null);

  const load = useCallback(async () => {
    if (!chatID) {
      setError("Invalid chat ID");
      return;
    }

    setError(null);
    try {
      const chatData = await getChat(chatID);
      setChat(chatData);
      setChatTitle(chatData.title);

      if (canManageChat(chatData)) {
        setMembers(await listChatMembers(chatID));
      } else {
        setMembers([]);
      }
    } catch (e: any) {
      setError(e?.message || "Unable to load chat settings");
    }
  }, [chatID]);

  useFocusEffect(
    useCallback(() => {
      void load();
    }, [load])
  );

  useEffect(() => {
    if (!showSavedBanner) {
      return;
    }

    const timeout = setTimeout(() => setShowSavedBanner(false), 2500);
    return () => clearTimeout(timeout);
  }, [showSavedBanner]);

  if (!isAuthenticated) {
    return <Redirect href="/login" />;
  }

  if (error) {
    return (
      <View style={commonStyles.centeredScreen}>
        <Text style={commonStyles.errorText}>{error}</Text>
        <TouchableOpacity style={commonStyles.retryButton} onPress={() => void load()}>
          <Text style={commonStyles.retryButtonText}>Retry</Text>
        </TouchableOpacity>
      </View>
    );
  }

  if (!chat) {
    return (
      <View style={commonStyles.centeredScreen}>
        <Text style={commonStyles.metaText}>Loading chat settings...</Text>
      </View>
    );
  }

  const isOwner = canManageChat(chat);

  const saveTitle = async () => {
    const normalized = chatTitle.trim();
    if (!normalized || normalized === chat.title) {
      return;
    }

    setBusy(true);
    try {
      await editChat(chat.chatID, normalized);
      await load();
      setShowSavedBanner(true);
    } catch (e: any) {
      Alert.alert("Unable to rename chat", e?.message || "Please retry.");
    } finally {
      setBusy(false);
    }
  };

  const addMemberByEmail = async () => {
    const normalizedEmail = memberEmailDraft.trim();
    if (!normalizedEmail) {
      setMemberError("Enter a member email.");
      return;
    }
    setMemberError(null);

    setBusy(true);
    try {
      const identity = await lookupUserByEmail(normalizedEmail);
      if (!identity?.userID) {
        setMemberError("No account exists for this email.");
        return;
      }
      await addChatMember(chat.chatID, identity.userID, "member");
      setMemberEmailDraft("");
      setMemberError(null);
      await load();
    } catch (e: any) {
      const apiError = getApiError(e);
      const normalized = apiError.message.toLowerCase();
      if (apiError.status === 404 || normalized.includes("user not found")) {
        setMemberError("No account exists for this email.");
      } else if (apiError.status === 409 || normalized.includes("already a member")) {
        setMemberError("User is already a member of this chat.");
      } else if (apiError.status === 400 || normalized.includes("invalid parameter: email")) {
        setMemberError("Enter a valid email address.");
      } else if (apiError.status === 403) {
        setMemberError("Only chat owners can add members.");
      } else {
        setMemberError(apiError.message || "Unable to add member. Please retry.");
      }
    } finally {
      setBusy(false);
    }
  };

  return (
    <KeyboardAwareScrollView
      style={commonStyles.screen}
      contentContainerStyle={commonStyles.screenContent}
      refreshControl={<RefreshControl refreshing={refreshing} onRefresh={() => {
        setRefreshing(true);
        void load().finally(() => setRefreshing(false));
      }} />}
    >
      <View style={commonStyles.sectionCard}>
        <Text style={commonStyles.sectionTitle}>Chat Title</Text>
        {showSavedBanner && (
          <View style={[commonStyles.feedbackBanner, commonStyles.successBanner]}>
            <Text style={commonStyles.successBannerText}>Chat title saved.</Text>
          </View>
        )}

        <TextInput
          style={commonStyles.input}
          value={chatTitle}
          editable={isOwner}
          onChangeText={setChatTitle}
        />
        {isOwner && (
          <TouchableOpacity
            style={[commonStyles.primaryButton, styles.saveTitleButton, busy && commonStyles.primaryButtonDisabled]}
            disabled={busy}
            onPress={() => void saveTitle()}
          >
            <Text style={commonStyles.primaryButtonText}>Save Title</Text>
          </TouchableOpacity>
        )}
      </View>

      {isOwner && (
        <View style={commonStyles.sectionCard}>
          <Text style={commonStyles.sectionTitle}>Members</Text>
          <Text style={[commonStyles.metaText, styles.metaTop]}>Invite by email</Text>

          <View style={styles.addRow}>
            <TextInput
              style={[commonStyles.input, styles.addInput]}
              value={memberEmailDraft}
              onChangeText={(nextValue) => {
                setMemberEmailDraft(nextValue);
                setMemberError(null);
              }}
              placeholder="name@company.com"
              placeholderTextColor={appColors.textSubtle}
              autoCapitalize="none"
              keyboardType="email-address"
            />
            <TouchableOpacity style={commonStyles.miniPrimaryButton} disabled={busy} onPress={() => void addMemberByEmail()}>
              <Text style={commonStyles.miniPrimaryButtonText}>Add</Text>
            </TouchableOpacity>
          </View>
          {memberError && <Text style={commonStyles.errorText}>{memberError}</Text>}

          {members.map((member) => (
            <View key={member.userID} style={[commonStyles.outlinedRow, styles.memberRow]}>
              <View style={styles.memberIdentityRow}>
                <Text style={commonStyles.emphasizedRowLabel}>{getIdentityLabel(member.userID)}</Text>
                <Text style={styles.memberRoleText}>({member.role})</Text>
              </View>
              <View style={commonStyles.inlineActionsRow}>
                {member.userID !== user?.userID && (
                  <TouchableOpacity
                    style={commonStyles.miniDangerButton}
                    onPress={() =>
                      Alert.alert("Remove member", "Remove this member from the chat?", [
                        { text: "Cancel", style: "cancel" },
                        {
                          text: "Remove",
                          style: "destructive",
                          onPress: () => void removeChatMember(chat.chatID, member.userID).then(load),
                        },
                      ])
                    }
                  >
                    <Text style={commonStyles.miniDangerButtonText}>Remove</Text>
                  </TouchableOpacity>
                )}
              </View>
            </View>
          ))}
        </View>
      )}

      {isOwner && (
        <TouchableOpacity
          style={[commonStyles.dangerBlockButton, busy && commonStyles.primaryButtonDisabled]}
          disabled={busy}
          onPress={() =>
            Alert.alert("Delete Chat", "Delete this chat and all messages/polls?", [
              { text: "Cancel", style: "cancel" },
              {
                text: "Delete",
                style: "destructive",
                onPress: async () => {
                  setBusy(true);
                  try {
                    await deleteChat(chat.chatID);
                    router.replace("/");
                  } catch (e: any) {
                    Alert.alert("Unable to delete chat", e?.message || "Please retry.");
                  } finally {
                    setBusy(false);
                  }
                },
              },
            ])
          }
        >
          <Text style={commonStyles.dangerBlockButtonText}>Delete Chat</Text>
        </TouchableOpacity>
      )}
    </KeyboardAwareScrollView>
  );
}

const styles = StyleSheet.create({
  metaTop: {
    marginTop: 4,
  },
  addRow: {
    flexDirection: "row",
    alignItems: "center",
    gap: 8,
    marginTop: 8,
    marginBottom: 8,
  },
  addInput: {
    flex: 1,
  },
  memberRow: {
    marginTop: 8,
    flexDirection: "row",
    justifyContent: "space-between",
    alignItems: "center",
  },
  memberIdentityRow: {
    flexDirection: "row",
    alignItems: "center",
    gap: 4,
    flexShrink: 1,
    paddingRight: 10,
  },
  memberRoleText: {
    color: appColors.textMuted,
    fontStyle: "italic",
  },
  saveTitleButton: {
    marginTop: 12,
  },
});
