#include "SubmitVote.h"

#include "../../Core.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"

#include <libstuff/libstuff.h>
#include <fmt/format.h>

namespace {

struct SubmitVoteRequestModel {
    int64_t pollID;
    int64_t optionID;

    static SubmitVoteRequestModel bind(const SData& request) {
        return {
            RequestBinding::requirePositiveInt64(request, "pollID"),
            RequestBinding::requirePositiveInt64(request, "optionID")
        };
    }
};

struct SubmitVoteResponseModel {
    string voteID;
    int64_t pollID;
    int64_t optionID;
    string createdAt;

    void writeTo(SData& response) const {
        ResponseBinding::setString(response, "voteID", voteID);
        ResponseBinding::setInt64(response, "pollID", pollID);
        ResponseBinding::setInt64(response, "optionID", optionID);
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
        STHROW("404 Poll not found");
    }

    // ---- 2. Verify the option belongs to this poll ----
    SQResult optionResult;
    const string optionQuery = fmt::format(
        "SELECT optionID FROM poll_options WHERE optionID = {} AND pollID = {};",
        input.optionID, input.pollID
    );

    if (!db.read(optionQuery, optionResult) || optionResult.empty()) {
        STHROW("400 Option does not belong to this poll");
    }

    // ---- 3. Insert the vote ----
    const string insertVote = fmt::format(
        "INSERT INTO votes (pollID, optionID, createdAt) VALUES ({}, {}, {});",
        input.pollID, input.optionID, createdAt
    );

    if (!db.write(insertVote)) {
        STHROW("502 Failed to insert vote");
    }

    // Get the vote ID
    SQResult idResult;
    if (!db.read("SELECT last_insert_rowid()", idResult) || idResult.empty() || idResult[0].empty()) {
        STHROW("502 Failed to retrieve voteID");
    }

    const SubmitVoteResponseModel output = {
        idResult[0][0],
        input.pollID,
        input.optionID,
        createdAt,
    };
    output.writeTo(response);

    SINFO("Vote " << idResult[0][0] << " cast on poll " << input.pollID << " for option " << input.optionID);
}
