#include "GetPollParticipation.h"

#include "PollCommandUtils.h"
#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"

#include <fmt/format.h>
#include <libstuff/libstuff.h>

#include <set>

namespace {

string boolToResponse(bool value) {
    return value ? "true" : "false";
}

struct GetPollParticipationRequestModel {
    int64_t pollID;
    int64_t requesterUserID; // Caller requesting participation visibility.

    static GetPollParticipationRequestModel bind(const SData& request) {
        return {
            RequestBinding::requirePositiveInt64(request, "pollID"),
            RequestBinding::requirePositiveInt64(request, "requesterUserID")
        };
    }
};

struct GetPollParticipationResponseModel {
    PollCommandUtils::PollRecord poll; // Source poll metadata for context fields.
    list<string> eligibleUserIDs; // Members of poll chat at read time.
    list<string> votedUserIDs; // Members detected as having submitted a response.
    list<string> notVotedUserIDs; // eligibleUserIDs minus votedUserIDs.

    void writeTo(SData& response) const {
        ResponseBinding::setInt64(response, "pollID", poll.pollID);
        ResponseBinding::setInt64(response, "chatID", poll.chatID);
        ResponseBinding::setString(response, "type", poll.type);
        ResponseBinding::setString(response, "isAnonymous", boolToResponse(poll.isAnonymous));

        ResponseBinding::setInt64(response, "eligibleCount", static_cast<int64_t>(eligibleUserIDs.size()));
        ResponseBinding::setInt64(response, "votedCount", static_cast<int64_t>(votedUserIDs.size()));
        ResponseBinding::setInt64(response, "notVotedCount", static_cast<int64_t>(notVotedUserIDs.size()));

        if (!poll.isAnonymous) {
            // Anonymous polls return only counts; user ID lists are included only for non-anonymous polls.
            ResponseBinding::setJSONArray(response, "eligibleUserIDs", eligibleUserIDs);
            ResponseBinding::setJSONArray(response, "votedUserIDs", votedUserIDs);
            ResponseBinding::setJSONArray(response, "notVotedUserIDs", notVotedUserIDs);
        }
    }
};

} // namespace

GetPollParticipation::GetPollParticipation(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool GetPollParticipation::peek(SQLite& db) {
    (void)db;
    (void)GetPollParticipationRequestModel::bind(request);
    return false;
}

void GetPollParticipation::process(SQLite& db) {
    const GetPollParticipationRequestModel input = GetPollParticipationRequestModel::bind(request);

    PollCommandUtils::PollRecord poll = PollCommandUtils::getPollOrThrow(
        db,
        input.pollID,
        "GetPollParticipation",
        "GET_POLL_PARTICIPATION_LOOKUP_FAILED",
        "GET_POLL_PARTICIPATION_NOT_FOUND"
    );

    PollCommandUtils::closePollIfExpired(
        db,
        poll,
        "GetPollParticipation",
        "GET_POLL_PARTICIPATION_CLOSE_EXPIRED_FAILED",
        "GET_POLL_PARTICIPATION_EVENT_CLOSE_INSERT_FAILED"
    );

    PollCommandUtils::requireChatMember(
        db,
        poll.chatID,
        input.requesterUserID,
        "GetPollParticipation",
        "GET_POLL_PARTICIPATION_CHAT_MEMBER_LOOKUP_FAILED",
        "GET_POLL_PARTICIPATION_REQUESTER_NOT_CHAT_MEMBER"
    );

    SQResult eligibleRows;
    const string eligibleQuery = fmt::format(
        "SELECT userID FROM chat_members WHERE chatID = {} ORDER BY userID ASC;",
        poll.chatID
    );
    if (!db.read(eligibleQuery, eligibleRows)) {
        CommandError::upstreamFailure(
            db,
            "Failed to read eligible users",
            "GET_POLL_PARTICIPATION_ELIGIBLE_LOOKUP_FAILED",
            {{"command", "GetPollParticipation"}, {"pollID", SToStr(poll.pollID)}}
        );
    }

    list<string> eligibleUserIDs;
    set<string> eligibleSet;
    for (const SQResultRow& row : eligibleRows) {
        if (row.empty()) {
            continue;
        }
        eligibleUserIDs.emplace_back(row[0]);
        eligibleSet.insert(row[0]);
    }

    SQResult votedRows;
    // Vote presence source depends on poll type:
    // - free_text polls use poll_text_responses
    // - choice/ranked polls use votes
    const string votedQuery = (poll.type == "free_text")
        ? fmt::format(
            "SELECT DISTINCT userID FROM poll_text_responses WHERE pollID = {} ORDER BY userID ASC;",
            poll.pollID
        )
        : fmt::format(
            "SELECT DISTINCT userID FROM votes WHERE pollID = {} ORDER BY userID ASC;",
            poll.pollID
        );

    if (!db.read(votedQuery, votedRows)) {
        CommandError::upstreamFailure(
            db,
            "Failed to read voted users",
            "GET_POLL_PARTICIPATION_VOTED_LOOKUP_FAILED",
            {{"command", "GetPollParticipation"}, {"pollID", SToStr(poll.pollID)}}
        );
    }

    list<string> votedUserIDs;
    set<string> votedSet;
    for (const SQResultRow& row : votedRows) {
        if (row.empty()) {
            continue;
        }
        // Count participation only for users who are still current members of this chat.
        if (!eligibleSet.contains(row[0])) {
            continue;
        }
        votedUserIDs.emplace_back(row[0]);
        votedSet.insert(row[0]);
    }

    list<string> notVotedUserIDs;
    for (const string& userID : eligibleUserIDs) {
        if (!votedSet.contains(userID)) {
            notVotedUserIDs.emplace_back(userID);
        }
    }

    const GetPollParticipationResponseModel output = {
        poll,
        eligibleUserIDs,
        votedUserIDs,
        notVotedUserIDs,
    };
    output.writeTo(response);
}
