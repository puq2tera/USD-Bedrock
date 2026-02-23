#include "RemoveChatMember.h"

#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"
#include "ChatAccess.h"

#include <fmt/format.h>
#include <libstuff/libstuff.h>

namespace {

struct RemoveChatMemberRequestModel {
    int64_t chatID;
    int64_t actingUserID; // Caller requesting the removal; must be an owner.
    int64_t userID;

    static RemoveChatMemberRequestModel bind(const SData& request) {
        return {
            RequestBinding::requirePositiveInt64(request, "chatID"),
            RequestBinding::requirePositiveInt64(request, "actingUserID"),
            RequestBinding::requirePositiveInt64(request, "userID"),
        };
    }
};

struct RemoveChatMemberResponseModel {
    int64_t chatID;
    int64_t userID;
    string result;

    void writeTo(SData& response) const {
        ResponseBinding::setInt64(response, "chatID", chatID);
        ResponseBinding::setInt64(response, "userID", userID);
        ResponseBinding::setString(response, "result", result);
    }
};

} // namespace

RemoveChatMember::RemoveChatMember(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool RemoveChatMember::peek(SQLite& db) {
    (void)db;
    (void)RemoveChatMemberRequestModel::bind(request);
    return false;
}

void RemoveChatMember::process(SQLite& db) {
    const RemoveChatMemberRequestModel input = RemoveChatMemberRequestModel::bind(request);

    ChatAccess::ensureChatExists(
        db,
        input.chatID,
        "RemoveChatMember",
        "REMOVE_CHAT_MEMBER_CHAT_LOOKUP_FAILED",
        "REMOVE_CHAT_MEMBER_CHAT_NOT_FOUND"
    );

    ChatAccess::requireOwner(
        db,
        input.chatID,
        input.actingUserID,
        "RemoveChatMember",
        "REMOVE_CHAT_MEMBER_ACTOR_LOOKUP_FAILED",
        "REMOVE_CHAT_MEMBER_FORBIDDEN"
    );

    const string existingRole = ChatAccess::requireMembershipRole(
        db,
        input.chatID,
        input.userID,
        "RemoveChatMember",
        "REMOVE_CHAT_MEMBER_TARGET_LOOKUP_FAILED",
        "REMOVE_CHAT_MEMBER_TARGET_NOT_MEMBER",
        "Target user is not a member of this chat"
    );

    if (existingRole == "owner") {
        const size_t owners = ChatAccess::ownerCount(
            db,
            input.chatID,
            "RemoveChatMember",
            "REMOVE_CHAT_MEMBER_OWNER_COUNT_FAILED"
        );
        if (owners <= 1) {
            CommandError::conflict(
                "A chat must have at least one owner",
                "REMOVE_CHAT_MEMBER_LAST_OWNER_CONFLICT",
                {{"command", "RemoveChatMember"}, {"chatID", SToStr(input.chatID)}}
            );
        }
    }

    const string deleteQuery = fmt::format(
        "DELETE FROM chat_members WHERE chatID = {} AND userID = {};",
        input.chatID,
        input.userID
    );
    if (!db.write(deleteQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to remove chat member",
            "REMOVE_CHAT_MEMBER_DELETE_FAILED",
            {{"command", "RemoveChatMember"}, {"chatID", SToStr(input.chatID)}, {"userID", SToStr(input.userID)}}
        );
    }

    const RemoveChatMemberResponseModel output = {input.chatID, input.userID, "removed"};
    output.writeTo(response);

    SINFO("Removed chat member " << input.userID << " from chat " << input.chatID);
}
