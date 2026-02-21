#include "DeletePoll.h"

#include "PollCommandUtils.h"
#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"

#include <fmt/format.h>
#include <libstuff/libstuff.h>

namespace {

struct DeletePollRequestModel {
    int64_t pollID;
    int64_t actorUserID;

    static DeletePollRequestModel bind(const SData& request) {
        return {
            RequestBinding::requirePositiveInt64(request, "pollID"),
            RequestBinding::requirePositiveInt64(request, "actorUserID")
        };
    }
};

struct DeletePollResponseModel {
    int64_t pollID;
    string result;

    void writeTo(SData& response) const {
        ResponseBinding::setInt64(response, "pollID", pollID);
        ResponseBinding::setString(response, "result", result);
    }
};

} // namespace

DeletePoll::DeletePoll(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool DeletePoll::peek(SQLite& db) {
    (void)db;
    (void)DeletePollRequestModel::bind(request);
    return false;
}

void DeletePoll::process(SQLite& db) {
    const DeletePollRequestModel input = DeletePollRequestModel::bind(request);

    PollCommandUtils::PollRecord poll = PollCommandUtils::getPollOrThrow(
        db,
        input.pollID,
        "DeletePoll",
        "DELETE_POLL_LOOKUP_FAILED",
        "DELETE_POLL_NOT_FOUND"
    );

    PollCommandUtils::closePollIfExpired(
        db,
        poll,
        "DeletePoll",
        "DELETE_POLL_CLOSE_EXPIRED_FAILED",
        "DELETE_POLL_EVENT_CLOSE_INSERT_FAILED"
    );

    PollCommandUtils::requireChatMember(
        db,
        poll.chatID,
        input.actorUserID,
        "DeletePoll",
        "DELETE_POLL_CHAT_MEMBER_LOOKUP_FAILED",
        "DELETE_POLL_ACTOR_NOT_CHAT_MEMBER"
    );

    if (poll.creatorUserID != input.actorUserID) {
        CommandError::conflict(
            "Only the poll creator can delete this poll",
            "DELETE_POLL_FORBIDDEN",
            {
                {"command", "DeletePoll"},
                {"pollID", SToStr(input.pollID)},
                {"actorUserID", SToStr(input.actorUserID)},
                {"creatorUserID", SToStr(poll.creatorUserID)}
            }
        );
    }

    PollCommandUtils::emitPollEvent(
        db,
        input.pollID,
        input.actorUserID,
        "deleted",
        {{"chatID", SToStr(poll.chatID)}},
        "DeletePoll",
        "DELETE_POLL_EVENT_INSERT_FAILED"
    );

    const string deletePollQuery = fmt::format(
        "DELETE FROM polls WHERE pollID = {};",
        input.pollID
    );
    if (!db.write(deletePollQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to delete poll",
            "DELETE_POLL_DELETE_FAILED",
            {{"command", "DeletePoll"}, {"pollID", SToStr(input.pollID)}}
        );
    }

    const DeletePollResponseModel output = {input.pollID, "deleted"};
    output.writeTo(response);

    SINFO("Deleted poll " << input.pollID);
}
