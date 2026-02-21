#include "EditChatMemberRole.h"

#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"
#include "ChatAccess.h"

#include <fmt/format.h>
#include <libstuff/libstuff.h>

namespace {

struct EditChatMemberRoleRequestModel {
    int64_t chatID;
    int64_t actingUserID;
    int64_t userID;
    string role;

    static EditChatMemberRoleRequestModel bind(const SData& request) {
        const int64_t chatID = RequestBinding::requirePositiveInt64(request, "chatID");
        const int64_t actingUserID = RequestBinding::requirePositiveInt64(request, "actingUserID");
        const int64_t userID = RequestBinding::requirePositiveInt64(request, "userID");
        const string role = ChatAccess::requireValidRole(
            RequestBinding::requireString(request, "role", 1, BedrockPlugin::MAX_SIZE_SMALL),
            "EditChatMemberRole",
            "EDIT_CHAT_MEMBER_ROLE_INVALID_ROLE"
        );

        return {chatID, actingUserID, userID, role};
    }
};

struct EditChatMemberRoleResponseModel {
    int64_t chatID;
    int64_t userID;
    string role;
    string result;

    void writeTo(SData& response) const {
        ResponseBinding::setInt64(response, "chatID", chatID);
        ResponseBinding::setInt64(response, "userID", userID);
        ResponseBinding::setString(response, "role", role);
        ResponseBinding::setString(response, "result", result);
    }
};

} // namespace

EditChatMemberRole::EditChatMemberRole(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool EditChatMemberRole::peek(SQLite& db) {
    (void)db;
    (void)EditChatMemberRoleRequestModel::bind(request);
    return false;
}

void EditChatMemberRole::process(SQLite& db) {
    const EditChatMemberRoleRequestModel input = EditChatMemberRoleRequestModel::bind(request);

    ChatAccess::ensureChatExists(
        db,
        input.chatID,
        "EditChatMemberRole",
        "EDIT_CHAT_MEMBER_ROLE_CHAT_LOOKUP_FAILED",
        "EDIT_CHAT_MEMBER_ROLE_CHAT_NOT_FOUND"
    );

    ChatAccess::requireOwner(
        db,
        input.chatID,
        input.actingUserID,
        "EditChatMemberRole",
        "EDIT_CHAT_MEMBER_ROLE_ACTOR_LOOKUP_FAILED",
        "EDIT_CHAT_MEMBER_ROLE_FORBIDDEN"
    );

    const string existingRole = ChatAccess::requireMembershipRole(
        db,
        input.chatID,
        input.userID,
        "EditChatMemberRole",
        "EDIT_CHAT_MEMBER_ROLE_TARGET_LOOKUP_FAILED",
        "EDIT_CHAT_MEMBER_ROLE_TARGET_NOT_MEMBER",
        "Target user is not a member of this chat"
    );

    if (existingRole == "owner" && input.role != "owner") {
        const size_t owners = ChatAccess::ownerCount(
            db,
            input.chatID,
            "EditChatMemberRole",
            "EDIT_CHAT_MEMBER_ROLE_OWNER_COUNT_FAILED"
        );
        if (owners <= 1) {
            CommandError::conflict(
                "A chat must have at least one owner",
                "EDIT_CHAT_MEMBER_ROLE_LAST_OWNER_CONFLICT",
                {{"command", "EditChatMemberRole"}, {"chatID", SToStr(input.chatID)}}
            );
        }
    }

    const string updateQuery = fmt::format(
        "UPDATE chat_members SET role = {} WHERE chatID = {} AND userID = {};",
        SQ(input.role),
        input.chatID,
        input.userID
    );
    if (!db.write(updateQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to update chat member role",
            "EDIT_CHAT_MEMBER_ROLE_UPDATE_FAILED",
            {{"command", "EditChatMemberRole"}, {"chatID", SToStr(input.chatID)}, {"userID", SToStr(input.userID)}}
        );
    }

    const EditChatMemberRoleResponseModel output = {
        input.chatID,
        input.userID,
        input.role,
        "updated",
    };
    output.writeTo(response);

    SINFO("Updated chat member role for user " << input.userID << " in chat " << input.chatID);
}
