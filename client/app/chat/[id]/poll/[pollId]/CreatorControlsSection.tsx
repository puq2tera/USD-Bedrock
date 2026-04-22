import { Switch, Text, TextInput, TouchableOpacity, View } from "react-native";
import { PollDetail } from "../../../../../lib/api";
import { commonStyles } from "../../../../../lib/styles";
import { pollDetailStyles as styles } from "./pollDetailStyles";

type CreatorControlsSectionProps = {
  poll: PollDetail;
  editing: boolean;
  editQuestion: string;
  editAllowChangeVote: boolean;
  editIsAnonymous: boolean;
  editExpiresAt: string;
  editOptions: string[];
  setEditing: (value: boolean) => void;
  setEditQuestion: (value: string) => void;
  setEditAllowChangeVote: (value: boolean) => void;
  setEditIsAnonymous: (value: boolean) => void;
  setEditExpiresAt: (value: string) => void;
  setEditOptions: (value: string[]) => void;
  onSubmitEdit: () => Promise<void>;
  onConfirmReplaceOptions: () => void;
  onTransitionStatus: (nextStatus: "open" | "closed") => void;
  onDeletePoll: () => void;
};

export function CreatorControlsSection(props: CreatorControlsSectionProps) {
  const {
    poll,
    editing,
    editQuestion,
    editAllowChangeVote,
    editIsAnonymous,
    editExpiresAt,
    editOptions,
    setEditing,
    setEditQuestion,
    setEditAllowChangeVote,
    setEditIsAnonymous,
    setEditExpiresAt,
    setEditOptions,
    onSubmitEdit,
    onConfirmReplaceOptions,
    onTransitionStatus,
    onDeletePoll,
  } = props;

  return (
    <View style={commonStyles.sectionCard}>
      <View style={commonStyles.sectionHeaderRow}>
        <Text style={commonStyles.sectionTitle}>Creator Controls</Text>
        <Switch value={editing} onValueChange={setEditing} />
      </View>

      {editing && (
        <>
          <Text style={commonStyles.sectionLabel}>Question</Text>
          <TextInput style={commonStyles.input} value={editQuestion} onChangeText={setEditQuestion} />

          <View style={commonStyles.formSwitchRow}>
            <Text style={commonStyles.formSwitchLabel}>Allow vote changes</Text>
            <Switch value={editAllowChangeVote} onValueChange={setEditAllowChangeVote} />
          </View>
          <View style={commonStyles.formSwitchRow}>
            <Text style={commonStyles.formSwitchLabel}>Anonymous</Text>
            <Switch value={editIsAnonymous} onValueChange={setEditIsAnonymous} />
          </View>

          <Text style={commonStyles.sectionLabel}>Expires at</Text>
          <TextInput style={commonStyles.input} value={editExpiresAt} onChangeText={setEditExpiresAt} placeholder="2026-05-01T18:00" />

          {poll.type !== "free_text" && (
            <>
              <Text style={commonStyles.sectionLabel}>Options</Text>
              {editOptions.map((label, index) => (
                <TextInput
                  key={`${index}-${label}`}
                  style={[commonStyles.input, styles.optionInputSpacing]}
                  value={label}
                  onChangeText={(nextLabel) => {
                    const next = [...editOptions];
                    next[index] = nextLabel;
                    setEditOptions(next);
                  }}
                />
              ))}
            </>
          )}

          <TouchableOpacity style={commonStyles.primaryButton} onPress={poll.type === "free_text" ? () => void onSubmitEdit() : onConfirmReplaceOptions}>
            <Text style={commonStyles.primaryButtonText}>Save poll updates</Text>
          </TouchableOpacity>
        </>
      )}

      <View style={[commonStyles.inlineActionsRow, styles.inlineActions]}>
        {poll.status === "open" ? (
          <TouchableOpacity style={commonStyles.miniDangerButton} onPress={() => onTransitionStatus("closed")}>
            <Text style={commonStyles.miniDangerButtonText}>Close poll</Text>
          </TouchableOpacity>
        ) : (
          <TouchableOpacity style={commonStyles.miniPrimaryButton} onPress={() => onTransitionStatus("open")}>
            <Text style={commonStyles.miniPrimaryButtonText}>Reopen poll</Text>
          </TouchableOpacity>
        )}

        <TouchableOpacity style={commonStyles.miniDangerButton} onPress={onDeletePoll}>
          <Text style={commonStyles.miniDangerButtonText}>Delete poll</Text>
        </TouchableOpacity>
      </View>
    </View>
  );
}
