#include "SubmitPollVotes.h"

#include "PollCommandUtils.h"
#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"

#include <fmt/format.h>
#include <libstuff/libstuff.h>
#include <set>
#include <vector>

namespace {

string boolToResponse(bool value) {
    return value ? "true" : "false";
}

struct SubmitPollVotesRequestModel {
    int64_t pollID;
    int64_t userID; // Caller submitting the vote set.
    vector<int64_t> optionIDs; // Submitted selection order; for ranked_choice this is rank 1..N.

    static SubmitPollVotesRequestModel bind(const SData& request) {
        const int64_t pollID = RequestBinding::requirePositiveInt64(request, "pollID");
        const int64_t userID = RequestBinding::requirePositiveInt64(request, "userID");

        const list<string> rawOptionIDs = RequestBinding::requireJSONArray(
            request,
            "optionIDs",
            1,
            PollCommandUtils::REQUEST_MAX_OPTIONS
        );
        vector<int64_t> optionIDs;
        set<int64_t> seen;
        for (const string& rawOptionID : rawOptionIDs) {
            const int64_t optionID = RequestBinding::parseInt64Strict(rawOptionID, "optionIDs");
            if (optionID <= 0) {
                CommandError::badRequest(
                    "Invalid parameter: optionIDs",
                    "SUBMIT_POLL_VOTES_INVALID_OPTION_ID",
                    {{"command", "SubmitPollVotes"}, {"parameter", "optionIDs"}}
                );
            }
            if (!seen.insert(optionID).second) {
                CommandError::badRequest(
                    "Duplicate option IDs are not allowed",
                    "SUBMIT_POLL_VOTES_DUPLICATE_OPTION_ID",
                    {{"command", "SubmitPollVotes"}, {"optionID", SToStr(optionID)}}
                );
            }
            optionIDs.emplace_back(optionID);
        }

        return {pollID, userID, optionIDs};
    }
};

struct SubmitPollVotesResponseModel {
    int64_t pollID;
    int64_t userID;
    list<string> optionIDs;
    int64_t submittedCount;
    bool replaced; // True when an existing vote set was overwritten.
    int64_t updatedAt;

    void writeTo(SData& response) const {
        ResponseBinding::setInt64(response, "pollID", pollID);
        ResponseBinding::setInt64(response, "userID", userID);
        ResponseBinding::setJSONArray(response, "optionIDs", optionIDs);
        ResponseBinding::setInt64(response, "submittedCount", submittedCount);
        ResponseBinding::setString(response, "replaced", boolToResponse(replaced));
        ResponseBinding::setInt64(response, "updatedAt", updatedAt);
    }
};

} // namespace

SubmitPollVotes::SubmitPollVotes(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool SubmitPollVotes::peek(SQLite& db) {
    (void)db;
    (void)SubmitPollVotesRequestModel::bind(request);
    return false;
}

void SubmitPollVotes::process(SQLite& db) {
    const SubmitPollVotesRequestModel input = SubmitPollVotesRequestModel::bind(request);

    PollCommandUtils::PollRecord poll = PollCommandUtils::getPollOrThrow(
        db,
        input.pollID,
        "SubmitPollVotes",
        "SUBMIT_POLL_VOTES_POLL_LOOKUP_FAILED",
        "SUBMIT_POLL_VOTES_POLL_NOT_FOUND"
    );

    PollCommandUtils::closePollIfExpired(
        db,
        poll,
        "SubmitPollVotes",
        "SUBMIT_POLL_VOTES_CLOSE_EXPIRED_FAILED",
        "SUBMIT_POLL_VOTES_EVENT_CLOSE_INSERT_FAILED"
    );

    PollCommandUtils::requireChatMember(
        db,
        poll.chatID,
        input.userID,
        "SubmitPollVotes",
        "SUBMIT_POLL_VOTES_CHAT_MEMBER_LOOKUP_FAILED",
        "SUBMIT_POLL_VOTES_USER_NOT_CHAT_MEMBER"
    );

    if (poll.status != "open") {
        CommandError::conflict(
            "Poll is closed",
            "SUBMIT_POLL_VOTES_POLL_CLOSED",
            {{"command", "SubmitPollVotes"}, {"pollID", SToStr(input.pollID)}}
        );
    }

    if (poll.type == "free_text") {
        CommandError::badRequest(
            "Poll does not accept option votes",
            "SUBMIT_POLL_VOTES_WRONG_TYPE",
            {{"command", "SubmitPollVotes"}, {"pollID", SToStr(input.pollID)}}
        );
    }

    if (poll.type == "single_choice" && input.optionIDs.size() != 1) {
        CommandError::badRequest(
            "Single-choice polls require exactly one option",
            "SUBMIT_POLL_VOTES_SINGLE_REQUIRES_ONE_OPTION",
            {{"command", "SubmitPollVotes"}, {"pollID", SToStr(input.pollID)}}
        );
    }

    SQResult optionResult;
    const string optionQuery = fmt::format(
        "SELECT optionID FROM poll_options WHERE pollID = {} AND isActive = 1;",
        input.pollID
    );

    if (!db.read(optionQuery, optionResult)) {
        CommandError::upstreamFailure(
            db,
            "Failed to verify poll options",
            "SUBMIT_POLL_VOTES_OPTIONS_LOOKUP_FAILED",
            {{"command", "SubmitPollVotes"}, {"pollID", SToStr(input.pollID)}}
        );
    }

    set<int64_t> validOptionIDs;
    for (const SQResultRow& row : optionResult) {
        if (!row.empty()) {
            validOptionIDs.insert(SToInt64(row[0]));
        }
    }

    for (const int64_t optionID : input.optionIDs) {
        if (!validOptionIDs.contains(optionID)) {
            CommandError::badRequest(
                "Option does not belong to this poll",
                "SUBMIT_POLL_VOTES_OPTION_NOT_IN_POLL",
                {
                    {"command", "SubmitPollVotes"},
                    {"pollID", SToStr(input.pollID)},
                    {"optionID", SToStr(optionID)}
                }
            );
        }
    }
    // Ranked-choice uses the caller's array order as rank, so all active options must be present exactly once.
    if (poll.type == "ranked_choice" && input.optionIDs.size() != validOptionIDs.size()) {
        CommandError::badRequest(
            "Ranked-choice polls require a complete ranking of all options",
            "SUBMIT_POLL_VOTES_RANKED_REQUIRES_ALL_OPTIONS",
            {
                {"command", "SubmitPollVotes"},
                {"pollID", SToStr(input.pollID)},
                {"submittedOptionCount", SToStr(input.optionIDs.size())},
                {"requiredOptionCount", SToStr(validOptionIDs.size())}
            }
        );
    }

    SQResult existingVotes;
    const string existingVotesQuery = fmt::format(
        "SELECT optionID FROM votes WHERE pollID = {} AND userID = {};",
        input.pollID,
        input.userID
    );
    if (!db.read(existingVotesQuery, existingVotes)) {
        CommandError::upstreamFailure(
            db,
            "Failed to verify existing votes",
            "SUBMIT_POLL_VOTES_EXISTING_LOOKUP_FAILED",
            {
                {"command", "SubmitPollVotes"},
                {"pollID", SToStr(input.pollID)},
                {"userID", SToStr(input.userID)}
            }
        );
    }

    const bool hasExistingVotes = !existingVotes.empty();
    if (hasExistingVotes && !poll.allowChangeVote) {
        CommandError::conflict(
            "Poll does not allow vote changes",
            "SUBMIT_POLL_VOTES_CHANGE_NOT_ALLOWED",
            {
                {"command", "SubmitPollVotes"},
                {"pollID", SToStr(input.pollID)},
                {"userID", SToStr(input.userID)}
            }
        );
    }

    if (hasExistingVotes) {
        // Vote updates are delete-then-insert so rank values are rebuilt as a clean contiguous sequence.
        const string deleteVotesQuery = fmt::format(
            "DELETE FROM votes WHERE pollID = {} AND userID = {};",
            input.pollID,
            input.userID
        );
        if (!db.write(deleteVotesQuery)) {
            CommandError::upstreamFailure(
                db,
                "Failed to replace existing votes",
                "SUBMIT_POLL_VOTES_REPLACE_DELETE_FAILED",
                {
                    {"command", "SubmitPollVotes"},
                    {"pollID", SToStr(input.pollID)},
                    {"userID", SToStr(input.userID)}
                }
            );
        }
    }

    const int64_t now = PollCommandUtils::nowUnix();
    size_t voteIndex = 0;
    list<string> responseOptionIDs;
    for (const int64_t optionID : input.optionIDs) {
        // `rank` stores caller order (1-based) for ranked/multiple polls.
        // Single-choice keeps rank as NULL.
        const string rankValue = (poll.type == "multiple_choice" || poll.type == "ranked_choice")
            ? SToStr(voteIndex + 1)
            : "NULL";
        const string insertVoteQuery = fmt::format(
            "INSERT INTO votes (pollID, optionID, userID, rank, createdAt, updatedAt) "
            "VALUES ({}, {}, {}, {}, {}, {});",
            input.pollID,
            optionID,
            input.userID,
            rankValue,
            now,
            now
        );

        if (!db.write(insertVoteQuery)) {
            CommandError::upstreamFailure(
                db,
                "Failed to submit vote",
                "SUBMIT_POLL_VOTES_INSERT_FAILED",
                {
                    {"command", "SubmitPollVotes"},
                    {"pollID", SToStr(input.pollID)},
                    {"optionID", SToStr(optionID)},
                    {"userID", SToStr(input.userID)}
                }
            );
        }

        responseOptionIDs.emplace_back(SToStr(optionID));
        ++voteIndex;
    }

    PollCommandUtils::emitPollEvent(
        db,
        input.pollID,
        input.userID,
        "votes_submitted",
        {
            {"userID", SToStr(input.userID)},
            {"optionCount", SToStr(input.optionIDs.size())},
            {"replaced", hasExistingVotes ? "true" : "false"}
        },
        "SubmitPollVotes",
        "SUBMIT_POLL_VOTES_EVENT_INSERT_FAILED"
    );

    const SubmitPollVotesResponseModel output = {
        input.pollID,
        input.userID,
        responseOptionIDs,
        static_cast<int64_t>(input.optionIDs.size()),
        hasExistingVotes,
        now,
    };
    output.writeTo(response);
}
