#include "PollOptionsTable.h"

#include "TableUtils.h"

#include <libstuff/libstuff.h>

namespace Tables::PollOptionsTable {

void verify(SQLite& db) {
    const string schema = R"(
        CREATE TABLE poll_options (
            optionID INTEGER PRIMARY KEY AUTOINCREMENT,
            pollID INTEGER NOT NULL,
            text TEXT NOT NULL,
            FOREIGN KEY (pollID) REFERENCES polls(pollID) ON DELETE CASCADE ON UPDATE CASCADE
        )
    )";

    TableUtils::verifyTableOrRecreate(db, "poll_options", schema);
    TableUtils::verifyIndex(db, "pollOptionsPollID", "poll_options", "(pollID)");
}

} // namespace Tables::PollOptionsTable
