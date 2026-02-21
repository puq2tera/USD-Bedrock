#include "PollCommandUtils.h"

#include <fmt/format.h>

#include <map>

namespace PollCommandUtils {

namespace {

optional<int64_t> parseNullableIntCell(const string& rawCell) {
    if (rawCell.empty()) {
        return nullopt;
    }
    return SToInt64(rawCell);
}

PollRecord rowToPollRecord(const SQResultRow& row) {
    if (row.size() < 12) {
        STHROW("Poll row missing required columns");
    }

    return {
        SToInt64(row[0]),
        SToInt64(row[1]),
        SToInt64(row[2]),
        row[3],
        row[4],
        row[5] == "1",
        row[6] == "1",
        row[7],
        parseNullableIntCell(row[8]),
        SToInt64(row[9]),
        SToInt64(row[10]),
        parseNullableIntCell(row[11]),
    };
}

void closePoll(SQLite& db,
               int64_t pollID,
               int64_t closedAt,
               const string& commandName,
               const string& closeErrorCode) {
    // Keep the original close timestamp if this path is retried.
    // We still refresh status/updatedAt so the row reflects the latest write attempt.
    const string closeQuery = fmt::format(
        "UPDATE polls "
        "SET status = 'closed', updatedAt = {}, closedAt = COALESCE(closedAt, {}) "
        "WHERE pollID = {} AND status = 'open';",
        closedAt,
        closedAt,
        pollID
    );

    if (!db.write(closeQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to close poll",
            closeErrorCode,
            {{"command", commandName}, {"pollID", SToStr(pollID)}}
        );
    }
}

string buildSummaryBody(SQLite& db,
                        const PollRecord& poll,
                        const string& commandName,
                        const string& readErrorCode) {
    if (poll.type == "free_text") {
        SQResult countResult;
        const string query = fmt::format(
            "SELECT COUNT(*) FROM poll_text_responses WHERE pollID = {};",
            poll.pollID
        );
        if (!db.read(query, countResult) || countResult.empty() || countResult[0].empty()) {
            CommandError::upstreamFailure(
                db,
                "Failed to summarize free-text poll responses",
                readErrorCode,
                {{"command", commandName}, {"pollID", SToStr(poll.pollID)}}
            );
        }

        return "Poll closed: " + poll.question + "\nResponses: " + countResult[0][0];
    }

    SQResult optionsResult;
    const string optionsQuery = fmt::format(
        "SELECT optionID, label FROM poll_options WHERE pollID = {} ORDER BY ord ASC, optionID ASC;",
        poll.pollID
    );
    if (!db.read(optionsQuery, optionsResult)) {
        CommandError::upstreamFailure(
            db,
            "Failed to summarize poll options",
            readErrorCode,
            {{"command", commandName}, {"pollID", SToStr(poll.pollID)}}
        );
    }

    SQResult countResult;
    // For ranked-choice polls, summary counts use only first-choice votes (rank = 1).
    const string countQuery = (poll.type == "ranked_choice")
        ? fmt::format(
            "SELECT optionID, COUNT(*) FROM votes WHERE pollID = {} AND rank = 1 GROUP BY optionID;",
            poll.pollID
        )
        : fmt::format(
            "SELECT optionID, COUNT(*) FROM votes WHERE pollID = {} GROUP BY optionID;",
            poll.pollID
        );
    if (!db.read(countQuery, countResult)) {
        CommandError::upstreamFailure(
            db,
            "Failed to summarize poll votes",
            readErrorCode,
            {{"command", commandName}, {"pollID", SToStr(poll.pollID)}}
        );
    }

    map<string, string> countsByOptionID;
    for (const SQResultRow& row : countResult) {
        if (row.size() >= 2) {
            countsByOptionID[row[0]] = row[1];
        }
    }

    string body = "Poll closed: " + poll.question + "\nType: " + poll.type + "\nResults:";
    for (const SQResultRow& row : optionsResult) {
        if (row.size() < 2) {
            continue;
        }

        const string& optionID = row[0];
        const string& optionLabel = row[1];
        const auto countIt = countsByOptionID.find(optionID);
        const string countValue = (countIt == countsByOptionID.end()) ? "0" : countIt->second;
        body += "\n- " + optionLabel + ": " + countValue;
    }
    return body;
}

} // namespace

int64_t nowUnix() {
    return static_cast<int64_t>(STimeNow());
}

bool parsePollType(const string& rawValue, string& normalizedType) {
    if (SIEquals(rawValue, "single_choice")) {
        normalizedType = "single_choice";
        return true;
    }
    if (SIEquals(rawValue, "multiple_choice")) {
        normalizedType = "multiple_choice";
        return true;
    }
    if (SIEquals(rawValue, "free_text")) {
        normalizedType = "free_text";
        return true;
    }
    if (SIEquals(rawValue, "ranked_choice")) {
        normalizedType = "ranked_choice";
        return true;
    }
    return false;
}

bool parsePollStatus(const string& rawValue, string& normalizedStatus) {
    if (SIEquals(rawValue, "open")) {
        normalizedStatus = "open";
        return true;
    }
    if (SIEquals(rawValue, "closed")) {
        normalizedStatus = "closed";
        return true;
    }
    return false;
}

PollRecord getPollOrThrow(SQLite& db,
                          int64_t pollID,
                          const string& commandName,
                          const string& lookupErrorCode,
                          const string& notFoundErrorCode) {
    SQResult pollResult;
    // IFNULL(..., '') preserves the project's "empty string means null optional timestamp" convention.
    const string pollQuery = fmt::format(
        "SELECT pollID, chatID, creatorUserID, question, type, allowChangeVote, isAnonymous, "
        "status, IFNULL(expiresAt, ''), createdAt, updatedAt, IFNULL(closedAt, '') "
        "FROM polls WHERE pollID = {};",
        pollID
    );

    if (!db.read(pollQuery, pollResult)) {
        CommandError::upstreamFailure(
            db,
            "Failed to query poll",
            lookupErrorCode,
            {{"command", commandName}, {"pollID", SToStr(pollID)}}
        );
    }

    if (pollResult.empty()) {
        CommandError::notFound(
            "Poll not found",
            notFoundErrorCode,
            {{"command", commandName}, {"pollID", SToStr(pollID)}}
        );
    }

    return rowToPollRecord(pollResult[0]);
}

void requireChatExists(SQLite& db,
                       int64_t chatID,
                       const string& commandName,
                       const string& lookupErrorCode,
                       const string& notFoundErrorCode) {
    SQResult chatResult;
    const string chatQuery = fmt::format(
        "SELECT chatID FROM chats WHERE chatID = {};",
        chatID
    );

    if (!db.read(chatQuery, chatResult)) {
        CommandError::upstreamFailure(
            db,
            "Failed to verify chat",
            lookupErrorCode,
            {{"command", commandName}, {"chatID", SToStr(chatID)}}
        );
    }

    if (chatResult.empty()) {
        CommandError::notFound(
            "Chat not found",
            notFoundErrorCode,
            {{"command", commandName}, {"chatID", SToStr(chatID)}}
        );
    }
}

void requireChatMember(SQLite& db,
                       int64_t chatID,
                       int64_t userID,
                       const string& commandName,
                       const string& lookupErrorCode,
                       const string& notMemberErrorCode) {
    SQResult membershipResult;
    const string membershipQuery = fmt::format(
        "SELECT 1 FROM chat_members WHERE chatID = {} AND userID = {} LIMIT 1;",
        chatID,
        userID
    );

    if (!db.read(membershipQuery, membershipResult)) {
        CommandError::upstreamFailure(
            db,
            "Failed to verify chat membership",
            lookupErrorCode,
            {
                {"command", commandName},
                {"chatID", SToStr(chatID)},
                {"userID", SToStr(userID)}
            }
        );
    }

    if (membershipResult.empty()) {
        CommandError::conflict(
            "User is not a member of this chat",
            notMemberErrorCode,
            {
                {"command", commandName},
                {"chatID", SToStr(chatID)},
                {"userID", SToStr(userID)}
            }
        );
    }
}

void emitPollEvent(SQLite& db,
                   int64_t pollID,
                   optional<int64_t> actorUserID,
                   const string& eventType,
                   const STable& payload,
                   const string& commandName,
                   const string& errorCode) {
    const string createdAt = SToStr(nowUnix());
    const string actorSQL = actorUserID ? SToStr(*actorUserID) : "NULL";
    const string payloadJson = SComposeJSONObject(payload);
    const string query = fmt::format(
        "INSERT INTO poll_events (pollID, actorUserID, eventType, payloadJSON, createdAt) "
        "VALUES ({}, {}, {}, {}, {});",
        pollID,
        actorSQL,
        SQ(eventType),
        SQ(payloadJson),
        createdAt
    );

    if (!db.write(query)) {
        CommandError::upstreamFailure(
            db,
            "Failed to persist poll event",
            errorCode,
            {{"command", commandName}, {"pollID", SToStr(pollID)}, {"eventType", eventType}}
        );
    }
}

bool closePollIfExpired(SQLite& db,
                        PollRecord& poll,
                        const string& commandName,
                        const string& closeErrorCode,
                        const string& eventErrorCode) {
    if (poll.status != "open" || !poll.expiresAt || *poll.expiresAt > nowUnix()) {
        return false;
    }

    const int64_t closedAt = nowUnix();
    closePoll(db, poll.pollID, closedAt, commandName, closeErrorCode);
    emitPollEvent(
        db,
        poll.pollID,
        nullopt,
        "closed",
        {{"reason", "expired"}},
        commandName,
        eventErrorCode
    );

    poll = getPollOrThrow(
        db,
        poll.pollID,
        commandName,
        closeErrorCode,
        "POLL_NOT_FOUND_AFTER_CLOSE"
    );
    (void)ensurePollSummaryMessage(
        db,
        poll,
        commandName,
        closeErrorCode,
        closeErrorCode
    );
    return true;
}

void autoCloseExpiredPollsInChat(SQLite& db,
                                 int64_t chatID,
                                 const string& commandName,
                                 const string& lookupErrorCode,
                                 const string& closeErrorCode,
                                 const string& eventErrorCode) {
    // Read endpoints call this first so responses do not show expired polls as still "open".
    const int64_t now = nowUnix();
    SQResult pollResult;
    const string query = fmt::format(
        "SELECT pollID "
        "FROM polls "
        "WHERE chatID = {} AND status = 'open' AND expiresAt IS NOT NULL AND expiresAt <= {};",
        chatID,
        now
    );

    if (!db.read(query, pollResult)) {
        CommandError::upstreamFailure(
            db,
            "Failed to evaluate poll expiry",
            lookupErrorCode,
            {{"command", commandName}, {"chatID", SToStr(chatID)}}
        );
    }

    for (const SQResultRow& row : pollResult) {
        if (row.empty()) {
            continue;
        }
        const int64_t pollID = SToInt64(row[0]);
        closePoll(db, pollID, now, commandName, closeErrorCode);
        emitPollEvent(
            db,
            pollID,
            nullopt,
            "closed",
            {{"reason", "expired"}},
            commandName,
            eventErrorCode
        );

        PollRecord poll = getPollOrThrow(
            db,
            pollID,
            commandName,
            closeErrorCode,
            "POLL_NOT_FOUND_AFTER_CLOSE"
        );
        (void)ensurePollSummaryMessage(
            db,
            poll,
            commandName,
            closeErrorCode,
            closeErrorCode
        );
    }
}

optional<int64_t> getPollSummaryMessageID(SQLite& db,
                                          int64_t pollID,
                                          const string& commandName,
                                          const string& errorCode) {
    SQResult summaryResult;
    const string query = fmt::format(
        "SELECT messageID FROM poll_summary_messages WHERE pollID = {};",
        pollID
    );
    if (!db.read(query, summaryResult)) {
        CommandError::upstreamFailure(
            db,
            "Failed to read poll summary message",
            errorCode,
            {{"command", commandName}, {"pollID", SToStr(pollID)}}
        );
    }
    if (summaryResult.empty() || summaryResult[0].empty()) {
        return nullopt;
    }
    return SToInt64(summaryResult[0][0]);
}

optional<int64_t> ensurePollSummaryMessage(SQLite& db,
                                           const PollRecord& poll,
                                           const string& commandName,
                                           const string& readErrorCode,
                                           const string& writeErrorCode) {
    if (poll.status != "closed") {
        return nullopt;
    }

    if (const optional<int64_t> existingMessageID = getPollSummaryMessageID(
            db,
            poll.pollID,
            commandName,
            readErrorCode)) {
        // If a summary was already created, reuse it instead of creating a duplicate.
        return existingMessageID;
    }

    const int64_t now = nowUnix();
    const string summaryBody = buildSummaryBody(db, poll, commandName, readErrorCode);
    const string insertMessageQuery = fmt::format(
        "INSERT INTO messages (chatID, userID, body, createdAt, updatedAt) VALUES ({}, {}, {}, {}, {});",
        poll.chatID,
        poll.creatorUserID,
        SQ(summaryBody),
        now,
        now
    );
    if (!db.write(insertMessageQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to insert poll summary message",
            writeErrorCode,
            {{"command", commandName}, {"pollID", SToStr(poll.pollID)}}
        );
    }

    SQResult messageIDResult;
    if (!db.read("SELECT last_insert_rowid()", messageIDResult) ||
        messageIDResult.empty() || messageIDResult[0].empty()) {
        CommandError::upstreamFailure(
            db,
            "Failed to read poll summary message ID",
            writeErrorCode,
            {{"command", commandName}, {"pollID", SToStr(poll.pollID)}}
        );
    }
    const int64_t messageID = SToInt64(messageIDResult[0][0]);

    const string insertSummaryLinkQuery = fmt::format(
        "INSERT INTO poll_summary_messages (pollID, messageID, createdAt) VALUES ({}, {}, {});",
        poll.pollID,
        messageID,
        now
    );
    if (!db.write(insertSummaryLinkQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to persist poll summary link",
            writeErrorCode,
            {{"command", commandName}, {"pollID", SToStr(poll.pollID)}, {"messageID", SToStr(messageID)}}
        );
    }

    return messageID;
}

string sqlNullableInt(const optional<int64_t>& value) {
    if (!value) {
        return "NULL";
    }
    return SToStr(*value);
}

string trimAndValidateText(const string& rawValue,
                           const char* fieldName,
                           size_t minLength,
                           size_t maxLength,
                           const string& errorCode,
                           const string& commandName) {
    const string trimmed = STrim(rawValue);
    if (trimmed.size() < minLength || trimmed.size() > maxLength) {
        CommandError::badRequest(
            string("Invalid parameter: ") + fieldName,
            errorCode,
            {{"command", commandName}, {"parameter", fieldName}}
        );
    }
    return trimmed;
}

} // namespace PollCommandUtils
