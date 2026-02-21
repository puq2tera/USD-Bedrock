#include "PollSummaryMessagesTable.h"

#include "../TableUtils.h"

#include <libstuff/libstuff.h>

namespace Tables::PollSummaryMessagesTable {

void verify(SQLite& db) {
    const string schema = R"(
        CREATE TABLE poll_summary_messages (
            pollID INTEGER PRIMARY KEY,
            messageID INTEGER NOT NULL UNIQUE,
            createdAt INTEGER NOT NULL,
            FOREIGN KEY (pollID) REFERENCES polls(pollID) ON DELETE CASCADE ON UPDATE CASCADE,
            FOREIGN KEY (messageID) REFERENCES messages(messageID) ON DELETE CASCADE ON UPDATE CASCADE,
            CHECK (createdAt > 0)
        )
    )";

    TableUtils::verifyTableOrRecreate(db, "poll_summary_messages", schema);
    TableUtils::verifyIndex(db, "pollSummaryMessagesMessageID", "poll_summary_messages", "(messageID)", true);
}

} // namespace Tables::PollSummaryMessagesTable
