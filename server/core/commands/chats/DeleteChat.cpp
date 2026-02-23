#include "DeleteChat.h"

#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"
#include "ChatAccess.h"

#include <fmt/format.h>
#include <libstuff/libstuff.h>

namespace {

struct DeleteChatRequestModel {
    int64_t chatID;
    int64_t userID; // Caller attempting deletion; must be an owner.

    static DeleteChatRequestModel bind(const SData& request) {
        return {
            RequestBinding::requirePositiveInt64(request, "chatID"),
            RequestBinding::requirePositiveInt64(request, "userID"),
        };
    }
};

struct DeleteChatResponseModel {
    int64_t chatID;
    string result;

    void writeTo(SData& response) const {
        ResponseBinding::setInt64(response, "chatID", chatID);
        ResponseBinding::setString(response, "result", result);
    }
};

} // namespace

DeleteChat::DeleteChat(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool DeleteChat::peek(SQLite& db) {
    (void)db;
    (void)DeleteChatRequestModel::bind(request);
    return false;
}

void DeleteChat::process(SQLite& db) {
    const DeleteChatRequestModel input = DeleteChatRequestModel::bind(request);

    ChatAccess::ensureChatExists(
        db,
        input.chatID,
        "DeleteChat",
        "DELETE_CHAT_LOOKUP_FAILED",
        "DELETE_CHAT_NOT_FOUND"
    );

    ChatAccess::requireOwner(
        db,
        input.chatID,
        input.userID,
        "DeleteChat",
        "DELETE_CHAT_MEMBER_LOOKUP_FAILED",
        "DELETE_CHAT_FORBIDDEN"
    );

    const string deleteQuery = fmt::format(
        "DELETE FROM chats WHERE chatID = {};",
        input.chatID
    );
    if (!db.write(deleteQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to delete chat",
            "DELETE_CHAT_DELETE_FAILED",
            {{"command", "DeleteChat"}, {"chatID", SToStr(input.chatID)}}
        );
    }

    const DeleteChatResponseModel output = {input.chatID, "deleted"};
    output.writeTo(response);

    SINFO("Deleted chat " << input.chatID);
}
