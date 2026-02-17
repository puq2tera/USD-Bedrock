#include "DeletePoll.h"

#include "../Core.h"

#include <libstuff/libstuff.h>
#include <fmt/format.h>

DeletePoll::DeletePoll(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool DeletePoll::peek(SQLite& db) {
    (void)db;
    validateRequest();
    return false; // Need the write phase
}

void DeletePoll::process(SQLite& db) {
    validateRequest();

    const string& pollID = request["pollID"];

    // ---- 1. Verify the poll exists ----
    SQResult pollResult;
    const string pollQuery = fmt::format(
        "SELECT pollID FROM polls WHERE pollID = {};",
        SQ(pollID)
    );

    if (!db.read(pollQuery, pollResult) || pollResult.empty()) {
        STHROW("404 Poll not found");
    }

    // ---- 2. Delete votes for this poll ----
    const string deleteVotes = fmt::format(
        "DELETE FROM votes WHERE pollID = {};",
        SQ(pollID)
    );

    if (!db.write(deleteVotes)) {
        STHROW("502 Failed to delete votes");
    }

    // ---- 3. Delete options for this poll ----
    const string deleteOptions = fmt::format(
        "DELETE FROM poll_options WHERE pollID = {};",
        SQ(pollID)
    );

    if (!db.write(deleteOptions)) {
        STHROW("502 Failed to delete poll options");
    }

    // ---- 4. Delete the poll itself ----
    const string deletePoll = fmt::format(
        "DELETE FROM polls WHERE pollID = {};",
        SQ(pollID)
    );

    if (!db.write(deletePoll)) {
        STHROW("502 Failed to delete poll");
    }

    response["pollID"] = pollID;
    response["result"] = "deleted";

    SINFO("Deleted poll " << pollID << " with its options and votes");
}

void DeletePoll::validateRequest() const {
    BedrockPlugin::verifyAttributeSize(request, "pollID", 1, BedrockPlugin::MAX_SIZE_SMALL);

    if (SToInt64(request["pollID"]) <= 0) {
        STHROW("400 pollID must be a positive integer");
    }
}
