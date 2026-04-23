import { Text, TextInput, TouchableOpacity, View } from "react-native";
import { canManageMessage, ChatMessage, getIdentityLabel } from "../../../lib/api";
import { chatDetailStyles as styles } from "./chatDetailStyles";
import { appColors, commonStyles } from "../../../lib/styles";

type MessagesSectionProps = {
  messages: ChatMessage[];
  editMessageID: string | null;
  editMessageBody: string;
  messageDraft: string;
  nextBeforeMessageID: string | null;
  loadingMoreMessages: boolean;
  busy: boolean;
  onChangeEditMessageBody: (value: string) => void;
  onChangeMessageDraft: (value: string) => void;
  onStartEditMessage: (message: ChatMessage) => void;
  onCancelEditMessage: () => void;
  onSaveEditedMessage: () => Promise<void>;
  onDeleteMessage: (messageID: string) => void;
  onLoadOlderMessages: () => Promise<void>;
  onSendMessage: () => Promise<void>;
};

export function MessagesSection(props: MessagesSectionProps) {
  const {
    messages,
    editMessageID,
    editMessageBody,
    messageDraft,
    nextBeforeMessageID,
    loadingMoreMessages,
    busy,
    onChangeEditMessageBody,
    onChangeMessageDraft,
    onStartEditMessage,
    onCancelEditMessage,
    onSaveEditedMessage,
    onDeleteMessage,
    onLoadOlderMessages,
    onSendMessage,
  } = props;

  return (
    <View style={commonStyles.sectionCard}>
      <View style={commonStyles.sectionHeaderRow}>
        <Text style={commonStyles.sectionTitle}>Messages</Text>
        <Text style={commonStyles.metaText}>{messages.length} loaded</Text>
      </View>

      {messages.map((message) => {
        const editable = canManageMessage(message);
        const editing = editMessageID === message.messageID;

        return (
          <View key={message.messageID} style={[commonStyles.outlinedRow, styles.messageRow]}>
            <Text style={[commonStyles.emphasizedRowLabel, styles.messageAuthor]}>{getIdentityLabel(message.userID)}</Text>
            {editing ? (
              <>
                <TextInput style={[commonStyles.input, styles.messageEditInput]} value={editMessageBody} onChangeText={onChangeEditMessageBody} multiline />
                <View style={commonStyles.inlineActionsRow}>
                  <TouchableOpacity style={commonStyles.ghostButton} onPress={onCancelEditMessage}>
                    <Text style={commonStyles.ghostButtonText}>Cancel</Text>
                  </TouchableOpacity>
                  <TouchableOpacity style={commonStyles.miniPrimaryButton} disabled={busy} onPress={() => void onSaveEditedMessage()}>
                    <Text style={commonStyles.miniPrimaryButtonText}>Save</Text>
                  </TouchableOpacity>
                </View>
              </>
            ) : (
              <Text style={styles.messageBody}>{message.body}</Text>
            )}

            {editable && !editing && (
              <View style={commonStyles.inlineActionsRow}>
                <TouchableOpacity style={commonStyles.ghostButton} onPress={() => onStartEditMessage(message)}>
                  <Text style={commonStyles.ghostButtonText}>Edit</Text>
                </TouchableOpacity>
                <TouchableOpacity style={commonStyles.miniDangerButton} onPress={() => onDeleteMessage(message.messageID)}>
                  <Text style={commonStyles.miniDangerButtonText}>Delete</Text>
                </TouchableOpacity>
              </View>
            )}
          </View>
        );
      })}

      {nextBeforeMessageID && (
        <TouchableOpacity style={commonStyles.ghostButton} disabled={loadingMoreMessages} onPress={() => void onLoadOlderMessages()}>
          <Text style={commonStyles.ghostButtonText}>{loadingMoreMessages ? "Loading..." : "Load Older Messages"}</Text>
        </TouchableOpacity>
      )}

      <TextInput
        style={[commonStyles.input, styles.messageComposer]}
        value={messageDraft}
        onChangeText={onChangeMessageDraft}
        placeholder="Write a message"
        placeholderTextColor={appColors.textSubtle}
        multiline
      />
      <TouchableOpacity style={commonStyles.primaryButton} disabled={busy} onPress={() => void onSendMessage()}>
        <Text style={commonStyles.primaryButtonText}>Send Message</Text>
      </TouchableOpacity>
    </View>
  );
}
