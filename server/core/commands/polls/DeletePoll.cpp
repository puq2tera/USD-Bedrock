#include "DeletePoll.h"

#include "../../Core.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"

#include <libstuff/libstuff.h>
#include <fmt/format.h>

namespace {

struct DeletePollRequestModel {
    int64_t pollID;

    static DeletePollRequestModel bind(const SData& request) {
        return {RequestBinding::requirePositiveInt64(request, "pollID")};
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
    return false; // Need the write phase
}

void DeletePoll::process(SQLite& db) {
    const DeletePollRequestModel input = DeletePollRequestModel::bind(request);

    // ---- 1. Verify the poll exists ----
    SQResult pollResult;
    const string pollQuery = fmt::format(
        "SELECT pollID FROM polls WHERE pollID = {};",
        input.pollID
    );

    if (!db.read(pollQuery, pollResult) || pollResult.empty()) {
        STHROW("404 Poll not found");
    }

    // ---- 2. Delete votes for this poll ----
    const string deleteVotes = fmt::format(
        "DELETE FROM votes WHERE pollID = {};",
        input.pollID
    );

    if (!db.write(deleteVotes)) {
        STHROW("502 Failed to delete votes");
    }

    // ---- 3. Delete options for this poll ----
    const string deleteOptions = fmt::format(
        "DELETE FROM poll_options WHERE pollID = {};",
        input.pollID
    );

    if (!db.write(deleteOptions)) {
        STHROW("502 Failed to delete poll options");
    }

    // ---- 4. Delete the poll itself ----
    const string deletePoll = fmt::format(
        "DELETE FROM polls WHERE pollID = {};",
        input.pollID
    );

    if (!db.write(deletePoll)) {
        STHROW("502 Failed to delete poll");
    }

    const DeletePollResponseModel output = {input.pollID, "deleted"};
    output.writeTo(response);

    SINFO("Deleted poll " << input.pollID << " with its options and votes");
}
