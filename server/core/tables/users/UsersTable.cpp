#include "UsersTable.h"

#include "../TableUtils.h"

#include <libstuff/libstuff.h>

namespace Tables::UsersTable {

void verify(SQLite& db) {
    const string schema = R"(
        CREATE TABLE users (
            userID INTEGER PRIMARY KEY AUTOINCREMENT,
            email TEXT NOT NULL COLLATE NOCASE UNIQUE,
            firstName TEXT NOT NULL,
            lastName TEXT NOT NULL,
            displayName TEXT NOT NULL,
            createdAt INTEGER NOT NULL,
            CHECK (length(trim(email)) BETWEEN 6 AND 254),
            CHECK (email = lower(email)),
            CHECK (instr(email, '@') > 1),
            CHECK (length(trim(firstName)) BETWEEN 1 AND 255),
            CHECK (length(trim(lastName)) BETWEEN 1 AND 255),
            CHECK (length(trim(displayName)) BETWEEN 1 AND 511),
            CHECK (createdAt > 0)
        )
    )";

    TableUtils::verifyTableOrRecreate(db, "users", schema);
}

} // namespace Tables::UsersTable
