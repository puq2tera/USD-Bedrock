#include "EditPoll.h"

#include "../../Core.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"

#include <libstuff/libstuff.h>
#include <fmt/format.h>

namespace {

struct EditPollRequestModel {
    int64_t pollID;
    optional<string> question;
    optional<list<string>> options;

    static EditPollRequestModel bind(const SData& request) {
        const int64_t pollID = RequestBinding::requirePositiveInt64(request, "pollID");
        const optional<string> question = RequestBinding::optionalString(request, "question", 1, BedrockPlugin::MAX_SIZE_SMALL);
        optional<list<string>> options = RequestBinding::optionalJSONArray(request, "options", 2, 20);

        if (options) {
            set<string> seen;
            for (const string& text : *options) {
                const string trimmed = SStrip(text);
                if (trimmed.empty()) {
                    STHROW("400 Option text cannot be empty");
                }
                if (!seen.insert(trimmed).second) {
                    STHROW("400 Duplicate option: " + trimmed);
                }
            }
        }

        if (!question && !options) {
            STHROW("400 Missing required parameter: question or options");
        }

        return {pollID, question, options};
    }
};

struct EditPollResponseModel {
    int64_t pollID;
    optional<size_t> optionCount;
    string result;

    void writeTo(SData& response) const {
        ResponseBinding::setInt64(response, "pollID", pollID);
        if (optionCount) {
            ResponseBinding::setSize(response, "optionCount", *optionCount);
        }
        ResponseBinding::setString(response, "result", result);
    }
};

} // namespace

EditPoll::EditPoll(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool EditPoll::peek(SQLite& db) {
    (void)db;
    (void)EditPollRequestModel::bind(request);
    return false; // Need the write phase
}

void EditPoll::process(SQLite& db) {
    const EditPollRequestModel input = EditPollRequestModel::bind(request);

    // ---- 1. Verify the poll exists ----
    SQResult pollResult;
    const string pollQuery = fmt::format(
        "SELECT pollID FROM polls WHERE pollID = {};",
        input.pollID
    );

    if (!db.read(pollQuery, pollResult) || pollResult.empty()) {
        STHROW("404 Poll not found");
    }

    optional<size_t> updatedOptionCount;

    // ---- 2. Update the question if provided ----
    if (input.question) {
        const string updateQuery = fmt::format(
            "UPDATE polls SET question = {} WHERE pollID = {};",
            SQ(*input.question), input.pollID
        );

        if (!db.write(updateQuery)) {
            STHROW("502 Failed to update poll question");
        }
    }

    // ---- 3. Replace options if provided ----
    if (input.options) {
        // Existing votes reference optionIDs; clear them before replacing options.
        const string deleteVotesQuery = fmt::format(
            "DELETE FROM votes WHERE pollID = {};",
            input.pollID
        );

        if (!db.write(deleteVotesQuery)) {
            STHROW("502 Failed to delete old votes");
        }

        // Delete existing options
        const string deleteQuery = fmt::format(
            "DELETE FROM poll_options WHERE pollID = {};",
            input.pollID
        );

        if (!db.write(deleteQuery)) {
            STHROW("502 Failed to delete old options");
        }

        // Insert new options
        for (const string& optionText : *input.options) {
            const string insertOption = fmt::format(
                "INSERT INTO poll_options (pollID, text) VALUES ({}, {});",
                input.pollID, SQ(optionText)
            );

            if (!db.write(insertOption)) {
                STHROW("502 Failed to insert poll option");
            }
        }

        updatedOptionCount = input.options->size();
    }

    // ---- 4. Build the response ----
    const EditPollResponseModel output = {
        input.pollID,
        updatedOptionCount,
        "updated",
    };
    output.writeTo(response);

    SINFO("Updated poll " << input.pollID);
}
