#include "PollOptionsTable.h"

#include "../TableUtils.h"

#include <libstuff/libstuff.h>

namespace Tables::PollOptionsTable {

void verify(SQLite& db) {
    const string schema = R"(
        CREATE TABLE poll_options (
            optionID INTEGER PRIMARY KEY AUTOINCREMENT,
            pollID INTEGER NOT NULL,
            label TEXT NOT NULL,
            ord INTEGER NOT NULL,
            isActive INTEGER NOT NULL,
            FOREIGN KEY (pollID) REFERENCES polls(pollID) ON DELETE CASCADE ON UPDATE CASCADE,
            CHECK (length(trim(label)) BETWEEN 1 AND 255),
            CHECK (ord >= 0),
            CHECK (isActive IN (0, 1)),
            UNIQUE (pollID, ord)
        )
    )";

    TableUtils::verifyTableOrRecreate(db, "poll_options", schema);
    TableUtils::verifyIndex(db, "pollOptionsPollID", "poll_options", "(pollID)");
}

} // namespace Tables::PollOptionsTable
