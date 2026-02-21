#include "SubmitVote.h"

#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"

#include <libstuff/libstuff.h>
#include <fmt/format.h>

namespace {

struct SubmitVoteRequestModel {
    int64_t pollID;
    int64_t optionID;
    int64_t userID;

    static SubmitVoteRequestModel bind(const SData& request) {
        return {
            RequestBinding::requirePositiveInt64(request, "pollID"),
            RequestBinding::requirePositiveInt64(request, "optionID"),
            RequestBinding::requirePositiveInt64(request, "userID")
        };
    }
};

struct SubmitVoteResponseModel {
    string voteID;
    int64_t pollID;
    int64_t optionID;
    int64_t userID;
    string createdAt;

    void writeTo(SData& response) const {
        ResponseBinding::setString(response, "voteID", voteID);
        ResponseBinding::setInt64(response, "pollID", pollID);
        ResponseBinding::setInt64(response, "optionID", optionID);
        ResponseBinding::setInt64(response, "userID", userID);
        ResponseBinding::setString(response, "createdAt", createdAt);
    }
};

} // namespace

SubmitVote::SubmitVote(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool SubmitVote::peek(SQLite& db) {
    (void)db;
    (void)SubmitVoteRequestModel::bind(request);
    return false; // Need the write phase to INSERT
}

void SubmitVote::process(SQLite& db) {
    const SubmitVoteRequestModel input = SubmitVoteRequestModel::bind(request);
    const string createdAt = SToStr(STimeNow());

    // ---- 1. Verify the poll exists ----
    SQResult pollResult;
    const string pollQuery = fmt::format(
        "SELECT pollID FROM polls WHERE pollID = {};",
        input.pollID
    );

    if (!db.read(pollQuery, pollResult) || pollResult.empty()) {
        CommandError::notFound(
            "Poll not found",
            "SUBMIT_VOTE_POLL_NOT_FOUND",
            {{"command", "SubmitVote"}, {"pollID", SToStr(input.pollID)}}
        );
    }

    // ---- 2. Verify the user exists ----
    SQResult userResult;
    const string userQuery = fmt::format(
        "SELECT userID FROM users WHERE userID = {};",
        input.userID
    );

    if (!db.read(userQuery, userResult) || userResult.empty()) {
        CommandError::notFound(
            "User not found",
            "SUBMIT_VOTE_USER_NOT_FOUND",
            {{"command", "SubmitVote"}, {"userID", SToStr(input.userID)}}
        );
    }

    // ---- 3. Verify the option belongs to this poll ----
    SQResult optionResult;
    const string optionQuery = fmt::format(
        "SELECT optionID FROM poll_options WHERE optionID = {} AND pollID = {};",
        input.optionID, input.pollID
    );

    if (!db.read(optionQuery, optionResult) || optionResult.empty()) {
        CommandError::badRequest(
            "Option does not belong to this poll",
            "SUBMIT_VOTE_OPTION_NOT_IN_POLL",
            {
                {"command", "SubmitVote"},
                {"pollID", SToStr(input.pollID)},
                {"optionID", SToStr(input.optionID)}
            }
        );
    }

    // ---- 4. Ensure user has not already voted in this poll ----
    SQResult existingVoteResult;
    const string existingVoteQuery = fmt::format(
        "SELECT voteID FROM votes WHERE pollID = {} AND userID = {} LIMIT 1;",
        input.pollID, input.userID
    );

    if (!db.read(existingVoteQuery, existingVoteResult)) {
        CommandError::upstreamFailure(
            db,
            "Failed to verify existing vote",
            "SUBMIT_VOTE_EXISTING_LOOKUP_FAILED",
            {
                {"command", "SubmitVote"},
                {"pollID", SToStr(input.pollID)},
                {"userID", SToStr(input.userID)}
            }
        );
    }
    if (!existingVoteResult.empty()) {
        CommandError::conflict(
            "User has already voted on this poll",
            "SUBMIT_VOTE_DUPLICATE_USER",
            {
                {"command", "SubmitVote"},
                {"pollID", SToStr(input.pollID)},
                {"userID", SToStr(input.userID)}
            }
        );
    }

    // ---- 5. Insert the vote ----
    const string insertVote = fmt::format(
        "INSERT INTO votes (pollID, optionID, userID, createdAt) VALUES ({}, {}, {}, {});",
        input.pollID, input.optionID, input.userID, createdAt
    );

    if (!db.write(insertVote)) {
        CommandError::upstreamFailure(
            db,
            "Failed to insert vote",
            "SUBMIT_VOTE_INSERT_FAILED",
            {
                {"command", "SubmitVote"},
                {"pollID", SToStr(input.pollID)},
                {"optionID", SToStr(input.optionID)},
                {"userID", SToStr(input.userID)}
            }
        );
    }

    // Get the vote ID
    SQResult idResult;
    if (!db.read("SELECT last_insert_rowid()", idResult) || idResult.empty() || idResult[0].empty()) {
        CommandError::upstreamFailure(
            db,
            "Failed to retrieve voteID",
            "SUBMIT_VOTE_LAST_INSERT_ID_FAILED",
            {
                {"command", "SubmitVote"},
                {"pollID", SToStr(input.pollID)},
                {"optionID", SToStr(input.optionID)},
                {"userID", SToStr(input.userID)}
            }
        );
    }

    const SubmitVoteResponseModel output = {
        idResult[0][0],
        input.pollID,
        input.optionID,
        input.userID,
        createdAt,
    };
    output.writeTo(response);

    SINFO("Vote " << idResult[0][0] << " cast on poll " << input.pollID << " for option " << input.optionID);
}
