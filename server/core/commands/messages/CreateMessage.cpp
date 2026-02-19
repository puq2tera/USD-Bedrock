#include "CreateMessage.h"

#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"

#include <libstuff/libstuff.h>
#include <fmt/format.h>

namespace {

struct CreateMessageRequestModel {
    int64_t userID;
    string name;
    string message;

    static CreateMessageRequestModel bind(const SData& request) {
        return {
            RequestBinding::requirePositiveInt64(request, "userID"),
            RequestBinding::requireString(request, "name", 1, BedrockPlugin::MAX_SIZE_SMALL),
            RequestBinding::requireString(request, "message", 1, BedrockPlugin::MAX_SIZE_QUERY),
        };
    }
};

struct CreateMessageResponseModel {
    string result;
    string messageID;
    int64_t userID;
    string name;
    string message;
    string createdAt;

    void writeTo(SData& response) const {
        ResponseBinding::setString(response, "result", result);
        ResponseBinding::setString(response, "messageID", messageID);
        ResponseBinding::setInt64(response, "userID", userID);
        ResponseBinding::setString(response, "name", name);
        ResponseBinding::setString(response, "message", message);
        ResponseBinding::setString(response, "createdAt", createdAt);
    }
};

} // namespace

CreateMessage::CreateMessage(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool CreateMessage::peek(SQLite& db) {
    (void)db;
    (void)CreateMessageRequestModel::bind(request);
    return false;
}

void CreateMessage::process(SQLite& db) {
    const CreateMessageRequestModel input = CreateMessageRequestModel::bind(request);
    const string createdAt = SToStr(STimeNow());

    SQResult userResult;
    const string userQuery = fmt::format(
        "SELECT userID FROM users WHERE userID = {};",
        input.userID
    );
    if (!db.read(userQuery, userResult)) {
        CommandError::upstreamFailure(
            db,
            "Failed to verify user",
            "CREATE_MESSAGE_USER_LOOKUP_FAILED",
            {{"command", "CreateMessage"}, {"userID", SToStr(input.userID)}}
        );
    }
    if (userResult.empty()) {
        CommandError::notFound(
            "User not found",
            "CREATE_MESSAGE_USER_NOT_FOUND",
            {{"command", "CreateMessage"}, {"userID", SToStr(input.userID)}}
        );
    }

    const string query = fmt::format(
        "INSERT INTO messages (userID, name, message, createdAt) VALUES ({}, {}, {}, {});",
        input.userID, SQ(input.name), SQ(input.message), createdAt
    );

    if (!db.write(query)) {
        CommandError::upstreamFailure(
            db,
            "Failed to insert message",
            "CREATE_MESSAGE_INSERT_FAILED",
            {{"command", "CreateMessage"}, {"userID", SToStr(input.userID)}}
        );
    }

    SQResult result;
    const string selectQuery = "SELECT last_insert_rowid()";
    if (!db.read(selectQuery, result) || result.empty() || result[0].empty()) {
        CommandError::upstreamFailure(
            db,
            "Failed to retrieve inserted messageID",
            "CREATE_MESSAGE_LAST_INSERT_ID_FAILED",
            {{"command", "CreateMessage"}, {"userID", SToStr(input.userID)}}
        );
    }

    const CreateMessageResponseModel output = {
        "stored",
        result[0][0],
        input.userID,
        input.name,
        input.message,
        createdAt,
    };
    output.writeTo(response);
}
