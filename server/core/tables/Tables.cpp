#include "Tables.h"

#include "chats/ChatMembersTable.h"
#include "chats/ChatsTable.h"
#include "chats/MessagesTable.h"
#include "polls/PollEventsTable.h"
#include "polls/PollOptionsTable.h"
#include "polls/PollSummaryMessagesTable.h"
#include "polls/PollTextResponsesTable.h"
#include "polls/PollsTable.h"
#include "polls/VotesTable.h"
#include "users/UsersTable.h"

namespace Tables {

void verifyAll(SQLite& db) {
    UsersTable::verify(db);
    ChatsTable::verify(db);
    ChatMembersTable::verify(db);
    MessagesTable::verify(db);
    PollsTable::verify(db);
    PollOptionsTable::verify(db);
    VotesTable::verify(db);
    PollTextResponsesTable::verify(db);
    PollEventsTable::verify(db);
    PollSummaryMessagesTable::verify(db);
}

} // namespace Tables
