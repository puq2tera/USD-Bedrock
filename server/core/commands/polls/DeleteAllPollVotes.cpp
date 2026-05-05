#include "DeleteAllPollVotes.h"

#include "PollCommandUtils.h"
#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"

#include <fmt/format.h>
#include <libstuff/libstuff.h>

namespace {

struct DeleteAllPollVotesRequestModel {
    int64_t pollID;
    int64_t actorUserID; // Caller requesting full reset; must be poll creator.

    static DeleteAllPollVotesRequestModel bind(const SData& request) {
        return {
            RequestBinding::requirePositiveInt64(request, "pollID"),
            RequestBinding::requirePositiveInt64(request, "actorUserID")
        };
    }
};

struct DeleteAllPollVotesResponseModel {
    int64_t pollID;
    int64_t removedVoteCount;
    int64_t removedTextResponseCount;

    void writeTo(SData& response) const {
        ResponseBinding::setInt64(response, "pollID", pollID);
        ResponseBinding::setInt64(response, "removedVoteCount", removedVoteCount);
        ResponseBinding::setInt64(response, "removedTextResponseCount", removedTextResponseCount);
    }
};

int64_t readCountOrThrow(SQLite& db, const string& query, const char* command, const char* errorCode, int64_t pollID) {
    SQResult countResult;
    if (!db.read(query, countResult) || countResult.empty() || countResult[0].empty()) {
        CommandError::upstreamFailure(
            db,
            "Failed to count poll participation rows",
            errorCode,
            {{"command", command}, {"pollID", SToStr(pollID)}}
        );
    }

    return SToInt64(countResult[0][0]);
}

} // namespace

DeleteAllPollVotes::DeleteAllPollVotes(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool DeleteAllPollVotes::peek(SQLite& db) {
    (void)db;
    (void)DeleteAllPollVotesRequestModel::bind(request);
    return false;
}

void DeleteAllPollVotes::process(SQLite& db) {
    const DeleteAllPollVotesRequestModel input = DeleteAllPollVotesRequestModel::bind(request);

    PollCommandUtils::PollRecord poll = PollCommandUtils::getPollOrThrow(
        db,
        input.pollID,
        "DeleteAllPollVotes",
        "DELETE_ALL_POLL_VOTES_POLL_LOOKUP_FAILED",
        "DELETE_ALL_POLL_VOTES_POLL_NOT_FOUND"
    );

    PollCommandUtils::closePollIfExpired(
        db,
        poll,
        "DeleteAllPollVotes",
        "DELETE_ALL_POLL_VOTES_CLOSE_EXPIRED_FAILED",
        "DELETE_ALL_POLL_VOTES_EVENT_CLOSE_INSERT_FAILED"
    );

    PollCommandUtils::requireChatMember(
        db,
        poll.chatID,
        input.actorUserID,
        "DeleteAllPollVotes",
        "DELETE_ALL_POLL_VOTES_CHAT_MEMBER_LOOKUP_FAILED",
        "DELETE_ALL_POLL_VOTES_ACTOR_NOT_CHAT_MEMBER"
    );

    if (poll.creatorUserID != input.actorUserID) {
        CommandError::conflict(
            "Only the poll creator can reset all votes",
            "DELETE_ALL_POLL_VOTES_FORBIDDEN",
            {
                {"command", "DeleteAllPollVotes"},
                {"pollID", SToStr(input.pollID)},
                {"actorUserID", SToStr(input.actorUserID)},
                {"creatorUserID", SToStr(poll.creatorUserID)}
            }
        );
    }

    const int64_t removedVoteCount = readCountOrThrow(
        db,
        fmt::format("SELECT COUNT(*) FROM votes WHERE pollID = {};", input.pollID),
        "DeleteAllPollVotes",
        "DELETE_ALL_POLL_VOTES_COUNT_FAILED",
        input.pollID
    );
    const int64_t removedTextResponseCount = readCountOrThrow(
        db,
        fmt::format("SELECT COUNT(*) FROM poll_text_responses WHERE pollID = {};", input.pollID),
        "DeleteAllPollVotes",
        "DELETE_ALL_POLL_TEXT_RESPONSES_COUNT_FAILED",
        input.pollID
    );

    if (!db.write(fmt::format("DELETE FROM votes WHERE pollID = {};", input.pollID))) {
        CommandError::upstreamFailure(
            db,
            "Failed to delete poll votes",
            "DELETE_ALL_POLL_VOTES_DELETE_FAILED",
            {{"command", "DeleteAllPollVotes"}, {"pollID", SToStr(input.pollID)}}
        );
    }

    if (!db.write(fmt::format("DELETE FROM poll_text_responses WHERE pollID = {};", input.pollID))) {
        CommandError::upstreamFailure(
            db,
            "Failed to delete poll text responses",
            "DELETE_ALL_POLL_TEXT_RESPONSES_DELETE_FAILED",
            {{"command", "DeleteAllPollVotes"}, {"pollID", SToStr(input.pollID)}}
        );
    }

    if (removedVoteCount > 0 || removedTextResponseCount > 0) {
        PollCommandUtils::emitPollEvent(
            db,
            input.pollID,
            input.actorUserID,
            "votes_reset_all",
            {
                {"actorUserID", SToStr(input.actorUserID)},
                {"removedVoteCount", SToStr(removedVoteCount)},
                {"removedTextResponseCount", SToStr(removedTextResponseCount)}
            },
            "DeleteAllPollVotes",
            "DELETE_ALL_POLL_VOTES_EVENT_INSERT_FAILED"
        );
    }

    const DeleteAllPollVotesResponseModel output = {
        input.pollID,
        removedVoteCount,
        removedTextResponseCount,
    };
    output.writeTo(response);
}
