#include "CreatePoll.h"

#include "../../Core.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"

#include <libstuff/libstuff.h>
#include <fmt/format.h>

namespace {

struct CreatePollRequestModel {
    string question;
    list<string> options;

    static CreatePollRequestModel bind(const SData& request) {
        auto opts = RequestBinding::requireJSONArray(request, "options", 2, 20);

        set<string> seen;
        for (const string& text : opts) {
            const string trimmed = SStrip(text);
            if (trimmed.empty()) {
                STHROW("400 Option text cannot be empty");
            }
            if (!seen.insert(trimmed).second) {
                STHROW("400 Duplicate option: " + trimmed);
            }
        }

        return {
            RequestBinding::requireString(request, "question", 1, BedrockPlugin::MAX_SIZE_SMALL),
            std::move(opts)
        };
    }
};

struct CreatePollResponseModel {
    string pollID;
    string question;
    size_t optionCount;
    string createdAt;

    void writeTo(SData& response) const {
        ResponseBinding::setString(response, "pollID", pollID);
        ResponseBinding::setString(response, "question", question);
        ResponseBinding::setSize(response, "optionCount", optionCount);
        ResponseBinding::setString(response, "createdAt", createdAt);
    }
};

} // namespace

CreatePoll::CreatePoll(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool CreatePoll::peek(SQLite& db) {
    (void)db;
    (void)CreatePollRequestModel::bind(request);
    return false; // false = "I need the write phase (process), don't stop here"
}

void CreatePoll::process(SQLite& db) {
    const CreatePollRequestModel input = CreatePollRequestModel::bind(request);
    const string createdAt = SToStr(STimeNow());

    // ---- 1. Insert the poll ----
    const string insertPoll = fmt::format(
        "INSERT INTO polls (question, createdAt) VALUES ({}, {});",
        SQ(input.question), createdAt
    );

    if (!db.write(insertPoll)) {
        STHROW("502 Failed to insert poll");
    }

    // Get the new poll's ID
    SQResult idResult;
    if (!db.read("SELECT last_insert_rowid()", idResult) || idResult.empty() || idResult[0].empty()) {
        STHROW("502 Failed to retrieve pollID");
    }
    const string pollID = idResult[0][0];

    // ---- 2. Insert each option ----
    for (const string& optionText : input.options) {
        const string insertOption = fmt::format(
            "INSERT INTO poll_options (pollID, text) VALUES ({}, {});",
            pollID, SQ(optionText)
        );

        if (!db.write(insertOption)) {
            STHROW("502 Failed to insert poll option");
        }
    }

    // ---- 3. Build the response ----
    const CreatePollResponseModel output = {
        pollID,
        input.question,
        input.options.size(),
        createdAt,
    };
    output.writeTo(response);

    SINFO("Created poll " << pollID << ": " << input.question);
}
