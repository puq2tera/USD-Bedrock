#include "Tables.h"

#include "MessagesTable.h"
#include "PollOptionsTable.h"
#include "PollsTable.h"
#include "UsersTable.h"
#include "VotesTable.h"

namespace Tables {

void verifyAll(SQLite& db) {
    UsersTable::verify(db);
    MessagesTable::verify(db);
    PollsTable::verify(db);
    PollOptionsTable::verify(db);
    VotesTable::verify(db);
}

} // namespace Tables
