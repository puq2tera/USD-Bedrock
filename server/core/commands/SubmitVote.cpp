#include "SubmitVote.h"

#include "../Core.h"

#include <libstuff/libstuff.h>
#include <fmt/format.h>

SubmitVote::SubmitVote(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool SubmitVote::peek(SQLite& db) {
    (void)db;
    validateRequest();
    return false; // Need the write phase to INSERT
}

void SubmitVote::process(SQLite& db) {
    validateRequest();

    const string& pollID = request["pollID"];
    const string& optionID = request["optionID"];
    const string createdAt = SToStr(STimeNow());

    // ---- 1. Verify the poll exists ----
    SQResult pollResult;
    const string pollQuery = fmt::format(
        "SELECT pollID FROM polls WHERE pollID = {};",
        SQ(pollID)
    );

    if (!db.read(pollQuery, pollResult) || pollResult.empty()) {
        STHROW("404 Poll not found");
    }

    // ---- 2. Verify the option belongs to this poll ----
    SQResult optionResult;
    const string optionQuery = fmt::format(
        "SELECT optionID FROM poll_options WHERE optionID = {} AND pollID = {};",
        SQ(optionID), SQ(pollID)
    );

    if (!db.read(optionQuery, optionResult) || optionResult.empty()) {
        STHROW("400 Option does not belong to this poll");
    }

    // ---- 3. Insert the vote ----
    const string insertVote = fmt::format(
        "INSERT INTO votes (pollID, optionID, createdAt) VALUES ({}, {}, {});",
        SQ(pollID), SQ(optionID), createdAt
    );

    if (!db.write(insertVote)) {
        STHROW("502 Failed to insert vote");
    }

    // Get the vote ID
    SQResult idResult;
    if (!db.read("SELECT last_insert_rowid()", idResult) || idResult.empty() || idResult[0].empty()) {
        STHROW("502 Failed to retrieve voteID");
    }

    response["voteID"] = idResult[0][0];
    response["pollID"] = pollID;
    response["optionID"] = optionID;
    response["createdAt"] = createdAt;

    SINFO("Vote " << idResult[0][0] << " cast on poll " << pollID << " for option " << optionID);
}

void SubmitVote::validateRequest() const {
    BedrockPlugin::verifyAttributeSize(request, "pollID", 1, BedrockPlugin::MAX_SIZE_SMALL);
    BedrockPlugin::verifyAttributeSize(request, "optionID", 1, BedrockPlugin::MAX_SIZE_SMALL);
}
