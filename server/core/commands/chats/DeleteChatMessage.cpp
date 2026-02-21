#include "DeleteChatMessage.h"

#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"
#include "ChatAccess.h"

#include <fmt/format.h>
#include <libstuff/libstuff.h>

namespace {

struct DeleteChatMessageRequestModel {
    int64_t chatID;
    int64_t messageID;
    int64_t userID; // Caller attempting deletion (author or chat owner).

    static DeleteChatMessageRequestModel bind(const SData& request) {
        return {
            RequestBinding::requirePositiveInt64(request, "chatID"),
            RequestBinding::requirePositiveInt64(request, "messageID"),
            RequestBinding::requirePositiveInt64(request, "userID"),
        };
    }
};

struct DeleteChatMessageResponseModel {
    int64_t messageID;
    int64_t chatID;
    string result;

    void writeTo(SData& response) const {
        ResponseBinding::setInt64(response, "messageID", messageID);
        ResponseBinding::setInt64(response, "chatID", chatID);
        ResponseBinding::setString(response, "result", result);
    }
};

} // namespace

DeleteChatMessage::DeleteChatMessage(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool DeleteChatMessage::peek(SQLite& db) {
    (void)db;
    (void)DeleteChatMessageRequestModel::bind(request);
    return false;
}

void DeleteChatMessage::process(SQLite& db) {
    const DeleteChatMessageRequestModel input = DeleteChatMessageRequestModel::bind(request);

    ChatAccess::ensureUserExists(
        db,
        input.userID,
        "DeleteChatMessage",
        "DELETE_CHAT_MESSAGE_USER_LOOKUP_FAILED",
        "DELETE_CHAT_MESSAGE_USER_NOT_FOUND"
    );

    ChatAccess::ensureChatExists(
        db,
        input.chatID,
        "DeleteChatMessage",
        "DELETE_CHAT_MESSAGE_CHAT_LOOKUP_FAILED",
        "DELETE_CHAT_MESSAGE_CHAT_NOT_FOUND"
    );

    SQResult messageResult;
    const string messageQuery = fmt::format(
        "SELECT messageID, userID FROM messages WHERE messageID = {} AND chatID = {} LIMIT 1;",
        input.messageID,
        input.chatID
    );

    if (!db.read(messageQuery, messageResult)) {
        CommandError::upstreamFailure(
            db,
            "Failed to verify chat message",
            "DELETE_CHAT_MESSAGE_LOOKUP_FAILED",
            {{"command", "DeleteChatMessage"}, {"chatID", SToStr(input.chatID)}, {"messageID", SToStr(input.messageID)}}
        );
    }

    if (messageResult.empty() || messageResult[0].size() < 2) {
        CommandError::notFound(
            "Message not found",
            "DELETE_CHAT_MESSAGE_NOT_FOUND",
            {{"command", "DeleteChatMessage"}, {"chatID", SToStr(input.chatID)}, {"messageID", SToStr(input.messageID)}}
        );
    }

    const int64_t messageAuthorUserID = SToInt64(messageResult[0][1]);
    const bool isAuthor = messageAuthorUserID == input.userID;

    if (!isAuthor) {
        // Non-authors may delete only if they are current chat owners.
        const optional<string> role = ChatAccess::optionalMembershipRole(
            db,
            input.chatID,
            input.userID,
            "DeleteChatMessage",
            "DELETE_CHAT_MESSAGE_ROLE_LOOKUP_FAILED"
        );
        if (!role || *role != "owner") {
            CommandError::throwError(
                403,
                "Only the message author or a chat owner can delete this message",
                "DELETE_CHAT_MESSAGE_FORBIDDEN",
                {{"command", "DeleteChatMessage"}, {"chatID", SToStr(input.chatID)}, {"messageID", SToStr(input.messageID)}}
            );
        }
    }

    const string deleteQuery = fmt::format(
        "DELETE FROM messages WHERE messageID = {} AND chatID = {};",
        input.messageID,
        input.chatID
    );
    if (!db.write(deleteQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to delete chat message",
            "DELETE_CHAT_MESSAGE_DELETE_FAILED",
            {{"command", "DeleteChatMessage"}, {"chatID", SToStr(input.chatID)}, {"messageID", SToStr(input.messageID)}}
        );
    }

    const DeleteChatMessageResponseModel output = {input.messageID, input.chatID, "deleted"};
    output.writeTo(response);

    SINFO("Deleted chat message " << input.messageID << " from chat " << input.chatID);
}
