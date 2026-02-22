#include "PollEventsTable.h"

#include "../TableUtils.h"

#include <libstuff/libstuff.h>

namespace Tables::PollEventsTable {

void verify(SQLite& db) {
    const string schema = R"(
        CREATE TABLE poll_events (
            eventID INTEGER PRIMARY KEY AUTOINCREMENT,
            pollID INTEGER NOT NULL,
            actorUserID INTEGER,
            eventType TEXT NOT NULL,
            payloadJSON TEXT,
            createdAt INTEGER NOT NULL,
            FOREIGN KEY (actorUserID) REFERENCES users(userID) ON DELETE SET NULL ON UPDATE CASCADE,
            CHECK (eventType IN (
                'created',
                'edited',
                'votes_submitted',
                'votes_removed',
                'text_submitted',
                'closed',
                'reopened',
                'deleted'
            )),
            CHECK (createdAt > 0)
        )
    )";

    TableUtils::verifyTableOrRecreate(db, "poll_events", schema);
    TableUtils::verifyIndex(db, "pollEventsPollID", "poll_events", "(pollID)");
    TableUtils::verifyIndex(db, "pollEventsActorUserID", "poll_events", "(actorUserID)");
}

} // namespace Tables::PollEventsTable
