#include "PollsTable.h"

#include "TableUtils.h"

#include <libstuff/libstuff.h>

namespace Tables::PollsTable {

void verify(SQLite& db) {
    const string schema = R"(
        CREATE TABLE polls (
            pollID INTEGER PRIMARY KEY AUTOINCREMENT,
            question TEXT NOT NULL,
            createdAt INTEGER NOT NULL,
            createdBy INTEGER NOT NULL,
            FOREIGN KEY (createdBy) REFERENCES users(userID) ON DELETE CASCADE ON UPDATE CASCADE
        )
    )";

    TableUtils::verifyTableOrRecreate(db, "polls", schema);
    TableUtils::verifyIndex(db, "pollsCreatedBy", "polls", "(createdBy)");
}

} // namespace Tables::PollsTable
