#include "DeleteUser.h"

#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"

#include <libstuff/libstuff.h>
#include <fmt/format.h>

namespace {

struct DeleteUserRequestModel {
    int64_t userID; // Target user record to delete.

    static DeleteUserRequestModel bind(const SData& request) {
        return {RequestBinding::requirePositiveInt64(request, "userID")};
    }
};

struct DeleteUserResponseModel {
    int64_t userID;
    string result;

    void writeTo(SData& response) const {
        ResponseBinding::setInt64(response, "userID", userID);
        ResponseBinding::setString(response, "result", result);
    }
};

} // namespace

DeleteUser::DeleteUser(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool DeleteUser::peek(SQLite& db) {
    (void)db;
    (void)DeleteUserRequestModel::bind(request);
    return false;
}

void DeleteUser::process(SQLite& db) {
    const DeleteUserRequestModel input = DeleteUserRequestModel::bind(request);

    SQResult userResult;
    const string existingUserQuery = fmt::format(
        "SELECT userID FROM users WHERE userID = {};",
        input.userID
    );
    if (!db.read(existingUserQuery, userResult)) {
        CommandError::upstreamFailure(
            db,
            "Failed to verify user",
            "DELETE_USER_LOOKUP_FAILED",
            {{"command", "DeleteUser"}, {"userID", SToStr(input.userID)}}
        );
    }
    if (userResult.empty()) {
        CommandError::notFound(
            "User not found",
            "DELETE_USER_NOT_FOUND",
            {{"command", "DeleteUser"}, {"userID", SToStr(input.userID)}}
        );
    }

    // We delete related data explicitly instead of relying on database-level cascade rules.
    // Order matters: delete child/dependent rows before parent polls/chats/users.
    const string deleteUserVotesQuery = fmt::format(
        "DELETE FROM votes WHERE userID = {};",
        input.userID
    );
    if (!db.write(deleteUserVotesQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to delete user votes",
            "DELETE_USER_VOTES_DELETE_FAILED",
            {{"command", "DeleteUser"}, {"userID", SToStr(input.userID)}}
        );
    }

    const string deletePollVotesQuery = fmt::format(
        // Remove votes cast on polls this user created (before deleting the polls themselves).
        "DELETE FROM votes WHERE pollID IN (SELECT pollID FROM polls WHERE creatorUserID = {});",
        input.userID
    );
    if (!db.write(deletePollVotesQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to delete votes for user polls",
            "DELETE_USER_POLL_VOTES_DELETE_FAILED",
            {{"command", "DeleteUser"}, {"userID", SToStr(input.userID)}}
        );
    }

    const string deletePollOptionsQuery = fmt::format(
        // Remove options for polls owned by this user to avoid dangling option rows.
        "DELETE FROM poll_options WHERE pollID IN (SELECT pollID FROM polls WHERE creatorUserID = {});",
        input.userID
    );
    if (!db.write(deletePollOptionsQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to delete options for user polls",
            "DELETE_USER_POLL_OPTIONS_DELETE_FAILED",
            {{"command", "DeleteUser"}, {"userID", SToStr(input.userID)}}
        );
    }

    const string deletePollEventsForOwnedPollsQuery = fmt::format(
        // Remove event history tied to polls owned by this user.
        "DELETE FROM poll_events WHERE pollID IN (SELECT pollID FROM polls WHERE creatorUserID = {});",
        input.userID
    );
    if (!db.write(deletePollEventsForOwnedPollsQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to delete poll events for user polls",
            "DELETE_USER_POLL_EVENTS_DELETE_FAILED",
            {{"command", "DeleteUser"}, {"userID", SToStr(input.userID)}}
        );
    }

    const string deleteUserPollEventsQuery = fmt::format(
        "DELETE FROM poll_events WHERE actorUserID = {};",
        input.userID
    );
    if (!db.write(deleteUserPollEventsQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to delete user poll events",
            "DELETE_USER_POLL_ACTOR_EVENTS_DELETE_FAILED",
            {{"command", "DeleteUser"}, {"userID", SToStr(input.userID)}}
        );
    }

    const string deletePollsQuery = fmt::format(
        "DELETE FROM polls WHERE creatorUserID = {};",
        input.userID
    );
    if (!db.write(deletePollsQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to delete user polls",
            "DELETE_USER_POLLS_DELETE_FAILED",
            {{"command", "DeleteUser"}, {"userID", SToStr(input.userID)}}
        );
    }

    const string deleteUserTextResponsesQuery = fmt::format(
        "DELETE FROM poll_text_responses WHERE userID = {};",
        input.userID
    );
    if (!db.write(deleteUserTextResponsesQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to delete user poll text responses",
            "DELETE_USER_POLL_TEXT_RESPONSES_DELETE_FAILED",
            {{"command", "DeleteUser"}, {"userID", SToStr(input.userID)}}
        );
    }

    const string deleteMessagesQuery = fmt::format(
        "DELETE FROM messages WHERE userID = {};",
        input.userID
    );
    if (!db.write(deleteMessagesQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to delete user messages",
            "DELETE_USER_MESSAGES_DELETE_FAILED",
            {{"command", "DeleteUser"}, {"userID", SToStr(input.userID)}}
        );
    }

    const string deleteUserChatMembershipsQuery = fmt::format(
        "DELETE FROM chat_members WHERE userID = {};",
        input.userID
    );
    if (!db.write(deleteUserChatMembershipsQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to delete user chat memberships",
            "DELETE_USER_CHAT_MEMBERSHIPS_DELETE_FAILED",
            {{"command", "DeleteUser"}, {"userID", SToStr(input.userID)}}
        );
    }

    const string deleteChatMembersForOwnedChatsQuery = fmt::format(
        // Clear memberships in chats owned by this user before deleting the chats.
        "DELETE FROM chat_members WHERE chatID IN (SELECT chatID FROM chats WHERE createdByUserID = {});",
        input.userID
    );
    if (!db.write(deleteChatMembersForOwnedChatsQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to delete members for user chats",
            "DELETE_USER_CHAT_MEMBERS_FOR_OWNED_CHATS_FAILED",
            {{"command", "DeleteUser"}, {"userID", SToStr(input.userID)}}
        );
    }

    const string deleteUserChatsQuery = fmt::format(
        "DELETE FROM chats WHERE createdByUserID = {};",
        input.userID
    );
    if (!db.write(deleteUserChatsQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to delete user chats",
            "DELETE_USER_CHATS_DELETE_FAILED",
            {{"command", "DeleteUser"}, {"userID", SToStr(input.userID)}}
        );
    }

    const string deleteUserQuery = fmt::format(
        "DELETE FROM users WHERE userID = {};",
        input.userID
    );
    if (!db.write(deleteUserQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to delete user",
            "DELETE_USER_DELETE_FAILED",
            {{"command", "DeleteUser"}, {"userID", SToStr(input.userID)}}
        );
    }

    const DeleteUserResponseModel output = {input.userID, "deleted"};
    output.writeTo(response);

    SINFO("Deleted user " << input.userID);
}
