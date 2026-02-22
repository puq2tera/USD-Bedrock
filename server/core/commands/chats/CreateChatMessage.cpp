#include "CreateChatMessage.h"

#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"
#include "ChatAccess.h"

#include <fmt/format.h>
#include <libstuff/libstuff.h>

namespace {

struct CreateChatMessageRequestModel {
    int64_t chatID;
    int64_t userID;
    string body;

    static CreateChatMessageRequestModel bind(const SData& request) {
        const int64_t chatID = RequestBinding::requirePositiveInt64(request, "chatID");
        const int64_t userID = RequestBinding::requirePositiveInt64(request, "userID");
        const string body = STrim(RequestBinding::requireString(request, "body", 1, BedrockPlugin::MAX_SIZE_QUERY));
        if (body.empty()) {
            CommandError::badRequest(
                "Invalid parameter: body",
                "CREATE_CHAT_MESSAGE_INVALID_BODY",
                {{"command", "CreateChatMessage"}, {"parameter", "body"}}
            );
        }

        return {chatID, userID, body};
    }
};

struct CreateChatMessageResponseModel {
    int64_t messageID;
    int64_t chatID;
    int64_t userID;
    string body;
    string createdAt;
    string updatedAt;
    string result;

    void writeTo(SData& response) const {
        ResponseBinding::setInt64(response, "messageID", messageID);
        ResponseBinding::setInt64(response, "chatID", chatID);
        ResponseBinding::setInt64(response, "userID", userID);
        ResponseBinding::setString(response, "body", body);
        ResponseBinding::setString(response, "createdAt", createdAt);
        ResponseBinding::setString(response, "updatedAt", updatedAt);
        ResponseBinding::setString(response, "result", result);
    }
};

} // namespace

CreateChatMessage::CreateChatMessage(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool CreateChatMessage::peek(SQLite& db) {
    (void)db;
    (void)CreateChatMessageRequestModel::bind(request);
    return false;
}

void CreateChatMessage::process(SQLite& db) {
    const CreateChatMessageRequestModel input = CreateChatMessageRequestModel::bind(request);
    const string now = SToStr(STimeNow());

    ChatAccess::ensureUserExists(
        db,
        input.userID,
        "CreateChatMessage",
        "CREATE_CHAT_MESSAGE_USER_LOOKUP_FAILED",
        "CREATE_CHAT_MESSAGE_USER_NOT_FOUND"
    );

    ChatAccess::ensureChatExists(
        db,
        input.chatID,
        "CreateChatMessage",
        "CREATE_CHAT_MESSAGE_CHAT_LOOKUP_FAILED",
        "CREATE_CHAT_MESSAGE_CHAT_NOT_FOUND"
    );

    (void)ChatAccess::requireMembershipRole(
        db,
        input.chatID,
        input.userID,
        "CreateChatMessage",
        "CREATE_CHAT_MESSAGE_MEMBER_LOOKUP_FAILED",
        "CREATE_CHAT_MESSAGE_FORBIDDEN"
    );

    const string insertQuery = fmt::format(
        "INSERT INTO messages (chatID, userID, body, createdAt, updatedAt) VALUES ({}, {}, {}, {}, {});",
        input.chatID,
        input.userID,
        SQ(input.body),
        now,
        now
    );
    if (!db.write(insertQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to insert chat message",
            "CREATE_CHAT_MESSAGE_INSERT_FAILED",
            {{"command", "CreateChatMessage"}, {"chatID", SToStr(input.chatID)}, {"userID", SToStr(input.userID)}}
        );
    }

    SQResult idResult;
    if (!db.read("SELECT last_insert_rowid()", idResult) || idResult.empty() || idResult[0].empty()) {
        CommandError::upstreamFailure(
            db,
            "Failed to retrieve inserted messageID",
            "CREATE_CHAT_MESSAGE_LAST_INSERT_ID_FAILED",
            {{"command", "CreateChatMessage"}, {"chatID", SToStr(input.chatID)}}
        );
    }

    const CreateChatMessageResponseModel output = {
        SToInt64(idResult[0][0]),
        input.chatID,
        input.userID,
        input.body,
        now,
        now,
        "stored",
    };
    output.writeTo(response);

    SINFO("Created chat message " << output.messageID << " in chat " << input.chatID);
}
