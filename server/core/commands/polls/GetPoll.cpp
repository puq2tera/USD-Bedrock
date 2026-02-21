#include "GetPoll.h"

#include "PollCommandUtils.h"
#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"

#include <fmt/format.h>
#include <libstuff/libstuff.h>
#include <map>

namespace {

string boolToResponse(bool value) {
    return value ? "true" : "false";
}

struct GetPollRequestModel {
    int64_t pollID;
    int64_t requesterUserID; // Caller requesting poll details.

    static GetPollRequestModel bind(const SData& request) {
        return {
            RequestBinding::requirePositiveInt64(request, "pollID"),
            RequestBinding::requirePositiveInt64(request, "requesterUserID")
        };
    }
};

struct GetPollResponseModel {
    PollCommandUtils::PollRecord poll;
    optional<int64_t> summaryMessageID; // Linked chat message summarizing close results, if created.
    list<string> options;
    list<string> responses;
    int64_t totalVotes; // Count of vote rows (or first-choice rows for ranked_choice).
    int64_t totalVoters; // Distinct users who voted via votes table.
    int64_t responseCount; // Count of free-text responses for free_text polls.

    void writeTo(SData& response) const {
        ResponseBinding::setInt64(response, "pollID", poll.pollID);
        ResponseBinding::setInt64(response, "chatID", poll.chatID);
        ResponseBinding::setInt64(response, "creatorUserID", poll.creatorUserID);
        ResponseBinding::setString(response, "question", poll.question);
        ResponseBinding::setString(response, "type", poll.type);
        ResponseBinding::setString(response, "allowChangeVote", boolToResponse(poll.allowChangeVote));
        ResponseBinding::setString(response, "isAnonymous", boolToResponse(poll.isAnonymous));
        ResponseBinding::setString(response, "status", poll.status);
        ResponseBinding::setString(response, "expiresAt", poll.expiresAt ? SToStr(*poll.expiresAt) : "");
        ResponseBinding::setInt64(response, "createdAt", poll.createdAt);
        ResponseBinding::setInt64(response, "updatedAt", poll.updatedAt);
        ResponseBinding::setString(response, "closedAt", poll.closedAt ? SToStr(*poll.closedAt) : "");
        ResponseBinding::setString(
            response,
            "summaryMessageID",
            summaryMessageID ? SToStr(*summaryMessageID) : ""
        );

        ResponseBinding::setJSONArray(response, "options", options);
        ResponseBinding::setJSONArray(response, "responses", responses);
        ResponseBinding::setInt64(response, "optionCount", static_cast<int64_t>(options.size()));
        ResponseBinding::setInt64(response, "totalVotes", totalVotes);
        ResponseBinding::setInt64(response, "totalVoters", totalVoters);
        ResponseBinding::setInt64(response, "responseCount", responseCount);
    }
};

} // namespace

GetPoll::GetPoll(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool GetPoll::peek(SQLite& db) {
    (void)db;
    (void)GetPollRequestModel::bind(request);
    return false;
}

void GetPoll::process(SQLite& db) {
    buildResponse(db);
}

void GetPoll::buildResponse(SQLite& db) {
    const GetPollRequestModel input = GetPollRequestModel::bind(request);

    PollCommandUtils::PollRecord poll = PollCommandUtils::getPollOrThrow(
        db,
        input.pollID,
        "GetPoll",
        "GET_POLL_LOOKUP_FAILED",
        "GET_POLL_NOT_FOUND"
    );

    PollCommandUtils::closePollIfExpired(
        db,
        poll,
        "GetPoll",
        "GET_POLL_CLOSE_EXPIRED_FAILED",
        "GET_POLL_EVENT_CLOSE_INSERT_FAILED"
    );

    PollCommandUtils::requireChatMember(
        db,
        poll.chatID,
        input.requesterUserID,
        "GetPoll",
        "GET_POLL_CHAT_MEMBER_LOOKUP_FAILED",
        "GET_POLL_REQUESTER_NOT_CHAT_MEMBER"
    );

    list<string> options;
    list<string> responses;
    int64_t totalVotes = 0;
    int64_t totalVoters = 0;
    int64_t responseCount = 0;
    const optional<int64_t> summaryMessageID = PollCommandUtils::getPollSummaryMessageID(
        db,
        poll.pollID,
        "GetPoll",
        "GET_POLL_SUMMARY_LOOKUP_FAILED"
    );

    if (poll.type == "single_choice" || poll.type == "multiple_choice" || poll.type == "ranked_choice") {
        SQResult optionResult;
        // Keep option listing stable by original author-defined order, then optionID as tie-breaker.
        const string optionQuery = fmt::format(
            "SELECT optionID, label, ord, isActive "
            "FROM poll_options WHERE pollID = {} "
            "ORDER BY ord ASC, optionID ASC;",
            poll.pollID
        );

        if (!db.read(optionQuery, optionResult)) {
            CommandError::upstreamFailure(
                db,
                "Failed to fetch poll options",
                "GET_POLL_OPTIONS_READ_FAILED",
                {{"command", "GetPoll"}, {"pollID", SToStr(poll.pollID)}}
            );
        }

        SQResult voteCountsResult;
        // For ranked-choice polls, each option's displayed voteCount uses first-choice votes only.
        const string voteCountsQuery = (poll.type == "ranked_choice")
            ? fmt::format(
                "SELECT optionID, COUNT(*) FROM votes WHERE pollID = {} AND rank = 1 GROUP BY optionID;",
                poll.pollID
            )
            : fmt::format(
                "SELECT optionID, COUNT(*) FROM votes WHERE pollID = {} GROUP BY optionID;",
                poll.pollID
            );
        if (!db.read(voteCountsQuery, voteCountsResult)) {
            CommandError::upstreamFailure(
                db,
                "Failed to fetch vote counts",
                "GET_POLL_VOTE_COUNTS_READ_FAILED",
                {{"command", "GetPoll"}, {"pollID", SToStr(poll.pollID)}}
            );
        }

        map<string, string> optionVoteCounts;
        for (const SQResultRow& row : voteCountsResult) {
            if (row.size() < 2) {
                continue;
            }
            optionVoteCounts[row[0]] = row[1];
            totalVotes += SToInt64(row[1]);
        }

        SQResult voterCountResult;
        // totalVoters is always distinct users from votes, even when totalVotes uses rank=1 for ranked polls.
        const string voterCountQuery = fmt::format(
            "SELECT COUNT(DISTINCT userID) FROM votes WHERE pollID = {};",
            poll.pollID
        );
        if (!db.read(voterCountQuery, voterCountResult) || voterCountResult.empty() || voterCountResult[0].empty()) {
            CommandError::upstreamFailure(
                db,
                "Failed to fetch voter count",
                "GET_POLL_VOTER_COUNT_READ_FAILED",
                {{"command", "GetPoll"}, {"pollID", SToStr(poll.pollID)}}
            );
        }
        totalVoters = SToInt64(voterCountResult[0][0]);

        for (const SQResultRow& row : optionResult) {
            if (row.size() < 4) {
                continue;
            }

            STable option;
            option["optionID"] = row[0];
            option["label"] = row[1];
            option["ord"] = row[2];
            option["isActive"] = (row[3] == "1") ? "true" : "false";

            auto countIt = optionVoteCounts.find(row[0]);
            option["voteCount"] = (countIt == optionVoteCounts.end()) ? "0" : countIt->second;
            options.emplace_back(SComposeJSONObject(option));
        }
    } else {
        SQResult responseResult;
        const string responseQuery = fmt::format(
            "SELECT responseID, userID, textValue, createdAt "
            "FROM poll_text_responses WHERE pollID = {} "
            "ORDER BY responseID ASC;",
            poll.pollID
        );

        if (!db.read(responseQuery, responseResult)) {
            CommandError::upstreamFailure(
                db,
                "Failed to fetch poll text responses",
                "GET_POLL_TEXT_RESPONSES_READ_FAILED",
                {{"command", "GetPoll"}, {"pollID", SToStr(poll.pollID)}}
            );
        }

        for (const SQResultRow& row : responseResult) {
            if (row.size() < 4) {
                continue;
            }

            STable responseRow;
            responseRow["responseID"] = row[0];
            if (!poll.isAnonymous) {
                // In anonymous polls we keep response text but hide who submitted it.
                responseRow["userID"] = row[1];
            }
            responseRow["textValue"] = row[2];
            responseRow["createdAt"] = row[3];
            responses.emplace_back(SComposeJSONObject(responseRow));
        }

        responseCount = static_cast<int64_t>(responses.size());
    }

    const GetPollResponseModel output = {
        poll,
        summaryMessageID,
        options,
        responses,
        totalVotes,
        totalVoters,
        responseCount,
    };
    output.writeTo(response);
}
