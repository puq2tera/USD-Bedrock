#include "DeleteUser.h"

#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"

#include <libstuff/libstuff.h>
#include <fmt/format.h>

namespace {

struct DeleteUserRequestModel {
    int64_t userID;

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
        "DELETE FROM votes WHERE pollID IN (SELECT pollID FROM polls WHERE createdBy = {});",
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
        "DELETE FROM poll_options WHERE pollID IN (SELECT pollID FROM polls WHERE createdBy = {});",
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

    const string deletePollsQuery = fmt::format(
        "DELETE FROM polls WHERE createdBy = {};",
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
