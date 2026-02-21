#include "EditChatMessage.h"

#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"
#include "ChatAccess.h"

#include <fmt/format.h>
#include <libstuff/libstuff.h>

namespace {

struct EditChatMessageRequestModel {
    int64_t chatID;
    int64_t messageID;
    int64_t userID; // Caller attempting edit (author or chat owner).
    string body;

    static EditChatMessageRequestModel bind(const SData& request) {
        const int64_t chatID = RequestBinding::requirePositiveInt64(request, "chatID");
        const int64_t messageID = RequestBinding::requirePositiveInt64(request, "messageID");
        const int64_t userID = RequestBinding::requirePositiveInt64(request, "userID");
        const string body = STrim(RequestBinding::requireString(request, "body", 1, BedrockPlugin::MAX_SIZE_QUERY));

        if (body.empty()) {
            CommandError::badRequest(
                "Invalid parameter: body",
                "EDIT_CHAT_MESSAGE_INVALID_BODY",
                {{"command", "EditChatMessage"}, {"parameter", "body"}}
            );
        }

        return {chatID, messageID, userID, body};
    }
};

struct EditChatMessageResponseModel {
    int64_t messageID;
    int64_t chatID;
    int64_t userID; // Original message author userID.
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

EditChatMessage::EditChatMessage(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool EditChatMessage::peek(SQLite& db) {
    (void)db;
    (void)EditChatMessageRequestModel::bind(request);
    return false;
}

void EditChatMessage::process(SQLite& db) {
    const EditChatMessageRequestModel input = EditChatMessageRequestModel::bind(request);
    const string now = SToStr(STimeNow());

    ChatAccess::ensureUserExists(
        db,
        input.userID,
        "EditChatMessage",
        "EDIT_CHAT_MESSAGE_USER_LOOKUP_FAILED",
        "EDIT_CHAT_MESSAGE_USER_NOT_FOUND"
    );

    ChatAccess::ensureChatExists(
        db,
        input.chatID,
        "EditChatMessage",
        "EDIT_CHAT_MESSAGE_CHAT_LOOKUP_FAILED",
        "EDIT_CHAT_MESSAGE_CHAT_NOT_FOUND"
    );

    SQResult messageResult;
    const string messageQuery = fmt::format(
        "SELECT messageID, userID, createdAt FROM messages WHERE messageID = {} AND chatID = {} LIMIT 1;",
        input.messageID,
        input.chatID
    );

    if (!db.read(messageQuery, messageResult)) {
        CommandError::upstreamFailure(
            db,
            "Failed to verify chat message",
            "EDIT_CHAT_MESSAGE_LOOKUP_FAILED",
            {{"command", "EditChatMessage"}, {"chatID", SToStr(input.chatID)}, {"messageID", SToStr(input.messageID)}}
        );
    }

    if (messageResult.empty() || messageResult[0].size() < 3) {
        CommandError::notFound(
            "Message not found",
            "EDIT_CHAT_MESSAGE_NOT_FOUND",
            {{"command", "EditChatMessage"}, {"chatID", SToStr(input.chatID)}, {"messageID", SToStr(input.messageID)}}
        );
    }

    const int64_t messageAuthorUserID = SToInt64(messageResult[0][1]);
    const bool isAuthor = messageAuthorUserID == input.userID;

    if (!isAuthor) {
        // Non-authors may edit only if they are current chat owners.
        const optional<string> role = ChatAccess::optionalMembershipRole(
            db,
            input.chatID,
            input.userID,
            "EditChatMessage",
            "EDIT_CHAT_MESSAGE_ROLE_LOOKUP_FAILED"
        );
        if (!role || *role != "owner") {
            CommandError::throwError(
                403,
                "Only the message author or a chat owner can edit this message",
                "EDIT_CHAT_MESSAGE_FORBIDDEN",
                {{"command", "EditChatMessage"}, {"chatID", SToStr(input.chatID)}, {"messageID", SToStr(input.messageID)}}
            );
        }
    }

    const string updateQuery = fmt::format(
        "UPDATE messages SET body = {}, updatedAt = {} WHERE messageID = {} AND chatID = {};",
        SQ(input.body),
        now,
        input.messageID,
        input.chatID
    );
    if (!db.write(updateQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to update chat message",
            "EDIT_CHAT_MESSAGE_UPDATE_FAILED",
            {{"command", "EditChatMessage"}, {"chatID", SToStr(input.chatID)}, {"messageID", SToStr(input.messageID)}}
        );
    }

    const EditChatMessageResponseModel output = {
        input.messageID,
        input.chatID,
        messageAuthorUserID,
        input.body,
        messageResult[0][2],
        now,
        "updated",
    };
    output.writeTo(response);

    SINFO("Updated chat message " << input.messageID << " in chat " << input.chatID);
}
