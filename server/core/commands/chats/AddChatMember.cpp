#include "AddChatMember.h"

#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"
#include "ChatAccess.h"

#include <fmt/format.h>
#include <libstuff/libstuff.h>

namespace {

struct AddChatMemberRequestModel {
    int64_t chatID;
    int64_t actingUserID; // Caller requesting the membership change; must be an owner.
    int64_t userID;
    string role;

    static AddChatMemberRequestModel bind(const SData& request) {
        const int64_t chatID = RequestBinding::requirePositiveInt64(request, "chatID");
        const int64_t actingUserID = RequestBinding::requirePositiveInt64(request, "actingUserID");
        const int64_t userID = RequestBinding::requirePositiveInt64(request, "userID");
        const optional<string> requestedRole = RequestBinding::optionalString(request, "role", 1, BedrockPlugin::MAX_SIZE_SMALL);
        const string role = ChatAccess::requireValidRole(
            requestedRole.value_or("member"),
            "AddChatMember",
            "ADD_CHAT_MEMBER_INVALID_ROLE"
        );

        return {chatID, actingUserID, userID, role};
    }
};

struct AddChatMemberResponseModel {
    int64_t chatID;
    int64_t userID;
    string role;
    string joinedAt; // Membership join timestamp for the inserted row.
    string result;

    void writeTo(SData& response) const {
        ResponseBinding::setInt64(response, "chatID", chatID);
        ResponseBinding::setInt64(response, "userID", userID);
        ResponseBinding::setString(response, "role", role);
        ResponseBinding::setString(response, "joinedAt", joinedAt);
        ResponseBinding::setString(response, "result", result);
    }
};

} // namespace

AddChatMember::AddChatMember(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool AddChatMember::peek(SQLite& db) {
    (void)db;
    (void)AddChatMemberRequestModel::bind(request);
    return false;
}

void AddChatMember::process(SQLite& db) {
    const AddChatMemberRequestModel input = AddChatMemberRequestModel::bind(request);
    const string now = SToStr(STimeNow());

    ChatAccess::ensureChatExists(
        db,
        input.chatID,
        "AddChatMember",
        "ADD_CHAT_MEMBER_CHAT_LOOKUP_FAILED",
        "ADD_CHAT_MEMBER_CHAT_NOT_FOUND"
    );

    ChatAccess::requireOwner(
        db,
        input.chatID,
        input.actingUserID,
        "AddChatMember",
        "ADD_CHAT_MEMBER_ACTOR_LOOKUP_FAILED",
        "ADD_CHAT_MEMBER_FORBIDDEN"
    );

    ChatAccess::ensureUserExists(
        db,
        input.userID,
        "AddChatMember",
        "ADD_CHAT_MEMBER_USER_LOOKUP_FAILED",
        "ADD_CHAT_MEMBER_USER_NOT_FOUND"
    );

    if (ChatAccess::optionalMembershipRole(
        db,
        input.chatID,
        input.userID,
        "AddChatMember",
        "ADD_CHAT_MEMBER_EXISTING_LOOKUP_FAILED"
    )) {
        CommandError::conflict(
            "User is already a member of this chat",
            "ADD_CHAT_MEMBER_ALREADY_EXISTS",
            {{"command", "AddChatMember"}, {"chatID", SToStr(input.chatID)}, {"userID", SToStr(input.userID)}}
        );
    }

    const string insertQuery = fmt::format(
        "INSERT INTO chat_members (chatID, userID, role, joinedAt) VALUES ({}, {}, {}, {});",
        input.chatID,
        input.userID,
        SQ(input.role),
        now
    );

    if (!db.write(insertQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to add chat member",
            "ADD_CHAT_MEMBER_INSERT_FAILED",
            {{"command", "AddChatMember"}, {"chatID", SToStr(input.chatID)}, {"userID", SToStr(input.userID)}}
        );
    }

    const AddChatMemberResponseModel output = {
        input.chatID,
        input.userID,
        input.role,
        now,
        "added",
    };
    output.writeTo(response);

    SINFO("Added chat member " << input.userID << " to chat " << input.chatID);
}
