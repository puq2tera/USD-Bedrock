#include "ListPolls.h"

#include "PollCommandUtils.h"
#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"

#include <fmt/format.h>
#include <libstuff/libstuff.h>

namespace {

struct ListPollsRequestModel {
    int64_t chatID;
    int64_t requesterUserID; // Caller requesting poll list.
    bool includeClosed; // When false, return only open polls.

    static ListPollsRequestModel bind(const SData& request) {
        return {
            RequestBinding::requirePositiveInt64(request, "chatID"),
            RequestBinding::requirePositiveInt64(request, "requesterUserID"),
            RequestBinding::optionalBool(request, "includeClosed").value_or(true)
        };
    }
};

struct ListPollsResponseModel {
    int64_t chatID;
    list<string> polls;

    void writeTo(SData& response) const {
        ResponseBinding::setInt64(response, "chatID", chatID);
        ResponseBinding::setInt64(response, "resultCount", static_cast<int64_t>(polls.size()));
        ResponseBinding::setJSONArray(response, "polls", polls);
    }
};

} // namespace

ListPolls::ListPolls(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool ListPolls::peek(SQLite& db) {
    (void)db;
    (void)ListPollsRequestModel::bind(request);
    return false;
}

void ListPolls::process(SQLite& db) {
    const ListPollsRequestModel input = ListPollsRequestModel::bind(request);

    PollCommandUtils::requireChatExists(
        db,
        input.chatID,
        "ListPolls",
        "LIST_POLLS_CHAT_LOOKUP_FAILED",
        "LIST_POLLS_CHAT_NOT_FOUND"
    );
    PollCommandUtils::requireChatMember(
        db,
        input.chatID,
        input.requesterUserID,
        "ListPolls",
        "LIST_POLLS_CHAT_MEMBER_LOOKUP_FAILED",
        "LIST_POLLS_REQUESTER_NOT_CHAT_MEMBER"
    );

    // Close expired polls first so the list response reflects current status.
    PollCommandUtils::autoCloseExpiredPollsInChat(
        db,
        input.chatID,
        "ListPolls",
        "LIST_POLLS_EXPIRED_LOOKUP_FAILED",
        "LIST_POLLS_CLOSE_EXPIRED_FAILED",
        "LIST_POLLS_EVENT_CLOSE_INSERT_FAILED"
    );

    SQResult pollResult;
    const string statusFilter = input.includeClosed ? "" : " AND p.status = 'open'";
    // Pull poll fields plus summary counters in one query to keep list responses fast and consistent.
    // Counters include optionCount, totalVotes, totalVoters, responseCount, and summaryMessageID.
    // For ranked-choice polls, totalVotes counts first-choice picks only (rank = 1).
    const string query = fmt::format(
        "SELECT p.pollID, p.creatorUserID, p.question, p.type, p.allowChangeVote, p.isAnonymous, p.status, "
        "IFNULL(p.expiresAt, ''), p.createdAt, p.updatedAt, IFNULL(p.closedAt, ''), "
        "(SELECT COUNT(*) FROM poll_options po WHERE po.pollID = p.pollID), "
        "(SELECT COUNT(*) FROM votes v WHERE v.pollID = p.pollID "
        "AND (p.type <> 'ranked_choice' OR IFNULL(v.rank, 0) = 1)), "
        "(SELECT COUNT(DISTINCT v.userID) FROM votes v WHERE v.pollID = p.pollID), "
        "(SELECT COUNT(*) FROM poll_text_responses r WHERE r.pollID = p.pollID), "
        "IFNULL((SELECT psm.messageID FROM poll_summary_messages psm WHERE psm.pollID = p.pollID), '') "
        "FROM polls p "
        "WHERE p.chatID = {}{} "
        "ORDER BY p.pollID DESC;",
        input.chatID,
        statusFilter
    );

    if (!db.read(query, pollResult)) {
        CommandError::upstreamFailure(
            db,
            "Failed to list polls",
            "LIST_POLLS_READ_FAILED",
            {{"command", "ListPolls"}, {"chatID", SToStr(input.chatID)}}
        );
    }

    list<string> polls;
    for (const SQResultRow& row : pollResult) {
        // Safety check: skip malformed rows if the result shape is not what we expect.
        if (row.size() < 16) {
            continue;
        }

        STable poll;
        poll["pollID"] = row[0];
        poll["chatID"] = SToStr(input.chatID);
        poll["creatorUserID"] = row[1];
        poll["question"] = row[2];
        poll["type"] = row[3];
        poll["allowChangeVote"] = (row[4] == "1") ? "true" : "false";
        poll["isAnonymous"] = (row[5] == "1") ? "true" : "false";
        poll["status"] = row[6];
        poll["expiresAt"] = row[7];
        poll["createdAt"] = row[8];
        poll["updatedAt"] = row[9];
        poll["closedAt"] = row[10];
        poll["optionCount"] = row[11];
        poll["totalVotes"] = row[12];
        poll["totalVoters"] = row[13];
        poll["responseCount"] = row[14];
        poll["summaryMessageID"] = row[15];

        polls.emplace_back(SComposeJSONObject(poll));
    }

    const ListPollsResponseModel output = {input.chatID, polls};
    output.writeTo(response);
}
