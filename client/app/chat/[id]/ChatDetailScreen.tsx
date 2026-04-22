import { useCallback } from "react";
import { ActivityIndicator, RefreshControl, ScrollView, Text, TouchableOpacity, View } from "react-native";
import { Redirect, useFocusEffect, useLocalSearchParams, useRouter } from "expo-router";
import TypescriptUtils from "../../../lib/TypescriptUtils";
import { getIdentityLabel } from "../../../lib/api";
import { useAuth } from "../../../lib/auth";
import { appColors, commonStyles } from "../../../lib/styles";
import { AdminSection } from "./AdminSection";
import { chatDetailStyles as styles } from "./chatDetailStyles";
import { MessagesSection } from "./MessagesSection";
import { PollsSection } from "./PollsSection";
import { useChatDetailState } from "./useChatDetailState";

export function ChatDetailScreen() {
  const { isAuthenticated } = useAuth();
  const router = useRouter();
  const { id } = useLocalSearchParams<{ id: string }>();
  const chatID = TypescriptUtils.parseString(id) ?? "";

  const state = useChatDetailState(chatID);

  useFocusEffect(
    useCallback(() => {
      void state.loadAll();
    }, [state.loadAll])
  );

  if (!isAuthenticated) {
    return <Redirect href="/login" />;
  }

  if (state.loading) {
    return (
      <View style={commonStyles.centeredScreen}>
        <ActivityIndicator color={appColors.accent} size="large" />
      </View>
    );
  }

  if (!state.chat || state.error) {
    return (
      <View style={commonStyles.centeredScreen}>
        <Text style={commonStyles.errorText}>{state.error || "Chat not found"}</Text>
        <TouchableOpacity style={commonStyles.retryButton} onPress={() => void state.loadAll()}>
          <Text style={commonStyles.retryButtonText}>Retry</Text>
        </TouchableOpacity>
      </View>
    );
  }

  return (
    <ScrollView
      style={commonStyles.screen}
      contentContainerStyle={commonStyles.screenContent}
      refreshControl={<RefreshControl refreshing={state.refreshing} onRefresh={() => {
        state.setRefreshing(true);
        void state.loadAll();
      }} />}
    >
      <View style={commonStyles.sectionCard}>
        <Text style={commonStyles.pageTitle}>{state.chat.title}</Text>
        <Text style={[commonStyles.metaText, styles.metaWithTop]}>Created by {getIdentityLabel(state.chat.createdByUserID)}</Text>
        <Text style={[commonStyles.metaText, styles.metaWithTop]}>Members: {state.chat.memberCount} (owners: {state.chat.ownerCount})</Text>
      </View>

      <MessagesSection
        messages={state.messages}
        editMessageID={state.editMessageID}
        editMessageBody={state.editMessageBody}
        messageDraft={state.messageDraft}
        nextBeforeMessageID={state.nextBeforeMessageID}
        loadingMoreMessages={state.loadingMoreMessages}
        busy={state.busy}
        onChangeEditMessageBody={state.setEditMessageBody}
        onChangeMessageDraft={state.setMessageDraft}
        onStartEditMessage={(message) => {
          state.setEditMessageID(message.messageID);
          state.setEditMessageBody(message.body);
        }}
        onCancelEditMessage={() => {
          state.setEditMessageID(null);
          state.setEditMessageBody("");
        }}
        onSaveEditedMessage={state.saveEditedMessage}
        onDeleteMessage={state.removeMessage}
        onLoadOlderMessages={state.loadOlderMessages}
        onSendMessage={state.createOrEditMessage}
      />

      <PollsSection
        polls={state.polls}
        onCreatePoll={() => router.push(`/chat/${state.chat?.chatID ?? chatID}/poll/create`)}
        onOpenPoll={(pollID) => router.push(`/chat/${state.chat?.chatID ?? chatID}/poll/${pollID}`)}
      />

      {state.isOwner && (
        <AdminSection
          chat={state.chat}
          members={state.members}
          busy={state.busy}
          newChatTitle={state.newChatTitle}
          memberUserIDDraft={state.memberUserIDDraft}
          onChangeNewChatTitle={state.setNewChatTitle}
          onChangeMemberUserIDDraft={state.setMemberUserIDDraft}
          onUpdateTitle={state.updateChatTitle}
          onAddMember={state.addMember}
          onToggleMemberRole={state.toggleMemberRole}
          onRemoveMember={state.removeMember}
          onDeleteChat={state.removeChat}
        />
      )}
    </ScrollView>
  );
}
