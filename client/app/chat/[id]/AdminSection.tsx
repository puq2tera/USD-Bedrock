import { Text, TextInput, TouchableOpacity, View } from "react-native";
import { ChatDetail, ChatMember, getIdentityLabel } from "../../../lib/api";
import { commonStyles } from "../../../lib/styles";
import { chatDetailStyles as styles } from "./chatDetailStyles";

type AdminSectionProps = {
  chat: ChatDetail;
  members: ChatMember[];
  busy: boolean;
  newChatTitle: string;
  memberUserIDDraft: string;
  onChangeNewChatTitle: (value: string) => void;
  onChangeMemberUserIDDraft: (value: string) => void;
  onUpdateTitle: () => Promise<void>;
  onAddMember: () => Promise<void>;
  onToggleMemberRole: (member: ChatMember) => Promise<void>;
  onRemoveMember: (member: ChatMember) => void;
  onDeleteChat: () => void;
};

export function AdminSection(props: AdminSectionProps) {
  const {
    chat,
    members,
    busy,
    newChatTitle,
    memberUserIDDraft,
    onChangeNewChatTitle,
    onChangeMemberUserIDDraft,
    onUpdateTitle,
    onAddMember,
    onToggleMemberRole,
    onRemoveMember,
    onDeleteChat,
  } = props;

  return (
    <View style={commonStyles.sectionCard}>
      <Text style={commonStyles.sectionTitle}>Admin</Text>

      <Text style={commonStyles.sectionLabel}>Chat Title</Text>
      <TextInput style={commonStyles.input} value={newChatTitle} onChangeText={onChangeNewChatTitle} />
      <TouchableOpacity style={commonStyles.miniPrimaryButton} onPress={() => void onUpdateTitle()} disabled={busy}>
        <Text style={commonStyles.miniPrimaryButtonText}>Update title</Text>
      </TouchableOpacity>

      <Text style={commonStyles.sectionLabel}>Members</Text>
      <View style={styles.inlineComposer}>
        <TextInput style={[commonStyles.input, styles.inlineInput]} value={memberUserIDDraft} onChangeText={onChangeMemberUserIDDraft} placeholder="User ID" />
        <TouchableOpacity style={commonStyles.miniPrimaryButton} onPress={() => void onAddMember()}>
          <Text style={commonStyles.miniPrimaryButtonText}>Add</Text>
        </TouchableOpacity>
      </View>

      {members.map((member) => (
        <View key={member.userID} style={[commonStyles.outlinedRow, styles.memberRow]}>
          <View>
            <Text style={[commonStyles.emphasizedRowLabel, styles.memberName]}>{getIdentityLabel(member.userID)}</Text>
            <Text style={commonStyles.metaText}>Role: {member.role}</Text>
          </View>
          <View style={commonStyles.inlineActionsRow}>
            <TouchableOpacity style={commonStyles.ghostButton} onPress={() => void onToggleMemberRole(member)}>
              <Text style={commonStyles.ghostButtonText}>{member.role === "owner" ? "Demote" : "Promote"}</Text>
            </TouchableOpacity>
            {member.userID !== chat.createdByUserID && (
              <TouchableOpacity style={commonStyles.miniDangerButton} onPress={() => onRemoveMember(member)}>
                <Text style={commonStyles.miniDangerButtonText}>Remove</Text>
              </TouchableOpacity>
            )}
          </View>
        </View>
      ))}

      <TouchableOpacity style={commonStyles.dangerBlockButton} onPress={onDeleteChat}>
        <Text style={commonStyles.dangerBlockButtonText}>Delete chat</Text>
      </TouchableOpacity>
    </View>
  );
}
