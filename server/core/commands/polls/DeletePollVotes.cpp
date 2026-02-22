#include "DeletePollVotes.h"

#include "PollCommandUtils.h"
#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"

#include <fmt/format.h>
#include <libstuff/libstuff.h>

namespace {

struct DeletePollVotesRequestModel {
    int64_t pollID;
    int64_t userID;

    static DeletePollVotesRequestModel bind(const SData& request) {
        return {
            RequestBinding::requirePositiveInt64(request, "pollID"),
            RequestBinding::requirePositiveInt64(request, "userID")
        };
    }
};

struct DeletePollVotesResponseModel {
    int64_t pollID;
    int64_t userID;
    int64_t removedCount;

    void writeTo(SData& response) const {
        ResponseBinding::setInt64(response, "pollID", pollID);
        ResponseBinding::setInt64(response, "userID", userID);
        ResponseBinding::setInt64(response, "removedCount", removedCount);
    }
};

} // namespace

DeletePollVotes::DeletePollVotes(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool DeletePollVotes::peek(SQLite& db) {
    (void)db;
    (void)DeletePollVotesRequestModel::bind(request);
    return false;
}

void DeletePollVotes::process(SQLite& db) {
    const DeletePollVotesRequestModel input = DeletePollVotesRequestModel::bind(request);

    PollCommandUtils::PollRecord poll = PollCommandUtils::getPollOrThrow(
        db,
        input.pollID,
        "DeletePollVotes",
        "DELETE_POLL_VOTES_POLL_LOOKUP_FAILED",
        "DELETE_POLL_VOTES_POLL_NOT_FOUND"
    );

    PollCommandUtils::closePollIfExpired(
        db,
        poll,
        "DeletePollVotes",
        "DELETE_POLL_VOTES_CLOSE_EXPIRED_FAILED",
        "DELETE_POLL_VOTES_EVENT_CLOSE_INSERT_FAILED"
    );

    PollCommandUtils::requireChatMember(
        db,
        poll.chatID,
        input.userID,
        "DeletePollVotes",
        "DELETE_POLL_VOTES_CHAT_MEMBER_LOOKUP_FAILED",
        "DELETE_POLL_VOTES_USER_NOT_CHAT_MEMBER"
    );

    if (poll.type == "free_text") {
        CommandError::badRequest(
            "Poll does not use option votes",
            "DELETE_POLL_VOTES_WRONG_TYPE",
            {{"command", "DeletePollVotes"}, {"pollID", SToStr(input.pollID)}}
        );
    }

    if (poll.status != "open") {
        CommandError::conflict(
            "Poll is closed",
            "DELETE_POLL_VOTES_POLL_CLOSED",
            {{"command", "DeletePollVotes"}, {"pollID", SToStr(input.pollID)}}
        );
    }

    SQResult voteCountResult;
    const string voteCountQuery = fmt::format(
        "SELECT COUNT(*) FROM votes WHERE pollID = {} AND userID = {};",
        input.pollID,
        input.userID
    );
    if (!db.read(voteCountQuery, voteCountResult) || voteCountResult.empty() || voteCountResult[0].empty()) {
        CommandError::upstreamFailure(
            db,
            "Failed to count existing votes",
            "DELETE_POLL_VOTES_COUNT_FAILED",
            {{"command", "DeletePollVotes"}, {"pollID", SToStr(input.pollID)}, {"userID", SToStr(input.userID)}}
        );
    }

    const int64_t removedCount = SToInt64(voteCountResult[0][0]);
    const string deleteVotesQuery = fmt::format(
        "DELETE FROM votes WHERE pollID = {} AND userID = {};",
        input.pollID,
        input.userID
    );
    if (!db.write(deleteVotesQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to delete poll votes",
            "DELETE_POLL_VOTES_DELETE_FAILED",
            {{"command", "DeletePollVotes"}, {"pollID", SToStr(input.pollID)}, {"userID", SToStr(input.userID)}}
        );
    }

    if (removedCount > 0) {
        PollCommandUtils::emitPollEvent(
            db,
            input.pollID,
            input.userID,
            "votes_removed",
            {{"userID", SToStr(input.userID)}, {"removedCount", SToStr(removedCount)}},
            "DeletePollVotes",
            "DELETE_POLL_VOTES_EVENT_INSERT_FAILED"
        );
    }

    const DeletePollVotesResponseModel output = {input.pollID, input.userID, removedCount};
    output.writeTo(response);
}
