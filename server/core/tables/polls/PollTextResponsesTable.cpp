#include "PollTextResponsesTable.h"

#include "../TableUtils.h"

#include <libstuff/libstuff.h>

namespace Tables::PollTextResponsesTable {

void verify(SQLite& db) {
    const string schema = R"(
        CREATE TABLE poll_text_responses (
            responseID INTEGER PRIMARY KEY AUTOINCREMENT,
            pollID INTEGER NOT NULL,
            userID INTEGER NOT NULL,
            textValue TEXT NOT NULL,
            createdAt INTEGER NOT NULL,
            FOREIGN KEY (pollID) REFERENCES polls(pollID) ON DELETE CASCADE ON UPDATE CASCADE,
            FOREIGN KEY (userID) REFERENCES users(userID) ON DELETE CASCADE ON UPDATE CASCADE,
            CHECK (length(trim(textValue)) BETWEEN 1 AND 10000),
            CHECK (createdAt > 0),
            UNIQUE (pollID, userID)
        )
    )";

    TableUtils::verifyTableOrRecreate(db, "poll_text_responses", schema);
    TableUtils::verifyIndex(db, "pollTextResponsesPollID", "poll_text_responses", "(pollID)");
    TableUtils::verifyIndex(db, "pollTextResponsesUserID", "poll_text_responses", "(userID)");
}

} // namespace Tables::PollTextResponsesTable
