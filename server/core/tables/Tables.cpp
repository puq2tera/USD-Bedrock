#include "Tables.h"

<<<<<<< HEAD
#include "chats/ChatMembersTable.h"
#include "chats/ChatsTable.h"
#include "chats/MessagesTable.h"
#include "polls/PollEventsTable.h"
#include "polls/PollOptionsTable.h"
#include "polls/PollTextResponsesTable.h"
#include "polls/PollsTable.h"
#include "polls/VotesTable.h"
#include "users/UsersTable.h"
=======
#include "MessagesTable.h"
#include "PollOptionsTable.h"
#include "PollsTable.h"
#include "UsersTable.h"
#include "VotesTable.h"
>>>>>>> origin/main

namespace Tables {

void verifyAll(SQLite& db) {
    UsersTable::verify(db);
<<<<<<< HEAD
    ChatsTable::verify(db);
    ChatMembersTable::verify(db);
=======
>>>>>>> origin/main
    MessagesTable::verify(db);
    PollsTable::verify(db);
    PollOptionsTable::verify(db);
    VotesTable::verify(db);
<<<<<<< HEAD
    PollTextResponsesTable::verify(db);
    PollEventsTable::verify(db);
=======
>>>>>>> origin/main
}

} // namespace Tables
