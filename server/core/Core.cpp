#include "Core.h"

#include "commands/messages/CreateMessage.h"
#include "commands/messages/GetMessages.h"
#include "commands/polls/CreatePoll.h"
#include "commands/polls/DeletePoll.h"
#include "commands/polls/EditPoll.h"
#include "commands/polls/GetPoll.h"
#include "commands/polls/GetPolls.h"
#include "commands/polls/SubmitVote.h"
#include "commands/system/HelloWorld.h"

#include <BedrockServer.h>

#undef SLOGPREFIX
#define SLOGPREFIX "{" << getName() << "} "

// Static member definitions
const string BedrockPlugin_Core::name("Core");

const string& BedrockPlugin_Core::getName() const {
    return name;
}

// Expose the appropriate function from our shared lib so bedrock can load it
extern "C" BedrockPlugin_Core* BEDROCK_PLUGIN_REGISTER_CORE(BedrockServer& s) {
    return new BedrockPlugin_Core(s);
}

BedrockPlugin_Core::BedrockPlugin_Core(BedrockServer& s) : BedrockPlugin(s) {
    // Initialize the plugin
}

BedrockPlugin_Core::~BedrockPlugin_Core() = default;

unique_ptr<BedrockCommand> BedrockPlugin_Core::getCommand(SQLiteCommand&& baseCommand) {
    // Check if this is a command we handle
    if (SIEquals(baseCommand.request.methodLine, "HelloWorld")) {
        return make_unique<HelloWorld>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "CreateMessage")) {
        return make_unique<CreateMessage>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "GetMessages")) {
        return make_unique<GetMessages>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "CreatePoll")) {
        return make_unique<CreatePoll>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "GetPoll")) {
        return make_unique<GetPoll>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "GetPolls")) {
        return make_unique<GetPolls>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "SubmitVote")) {
        return make_unique<SubmitVote>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "EditPoll")) {
        return make_unique<EditPoll>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "DeletePoll")) {
        return make_unique<DeletePoll>(std::move(baseCommand), this);
    }

    // Not our command
    return nullptr;
}

const string& BedrockPlugin_Core::getVersion() const {
    static const string version = "1.1.0";
    return version;
}

STable BedrockPlugin_Core::getInfo() {
    STable info;
    info["name"] = getName();
    info["version"] = getVersion();
    return info;
}

bool BedrockPlugin_Core::shouldLockCommitPageOnTableConflict(const string& tableName) const {
    // Use default behavior (return false)
    (void)tableName; // Unused
    return false;
}

void BedrockPlugin_Core::upgradeDatabase(SQLite& db) {
    bool created = false;
    const string messagesTableSchema = R"(
        CREATE TABLE messages (
            messageID INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            message TEXT NOT NULL,
            createdAt INTEGER NOT NULL
        )
    )";

    while (!db.verifyTable("messages", messagesTableSchema, created)) {
        SASSERT(db.write("DROP TABLE messages"));
    }

    // Helpful index for fetching the most recent messages
    db.verifyIndex("messagesCreatedAt", "messages",
                   "(createdAt DESC)",
                   false, true);

    // ------------------------------------------------------------------
    // Polls tables
    // ------------------------------------------------------------------

    // The main polls table — one row per poll
    const string pollsTableSchema = R"(
        CREATE TABLE polls (
            pollID INTEGER PRIMARY KEY AUTOINCREMENT,
            question TEXT NOT NULL,
            createdAt INTEGER NOT NULL
        )
    )";

    created = false;
    while (!db.verifyTable("polls", pollsTableSchema, created)) {
        SASSERT(db.write("DROP TABLE polls"));
    }

    // Each poll has multiple options to vote on
    const string pollOptionsTableSchema = R"(
        CREATE TABLE poll_options (
            optionID INTEGER PRIMARY KEY AUTOINCREMENT,
            pollID INTEGER NOT NULL,
            text TEXT NOT NULL,
            FOREIGN KEY (pollID) REFERENCES polls(pollID)
        )
    )";

    created = false;
    while (!db.verifyTable("poll_options", pollOptionsTableSchema, created)) {
        SASSERT(db.write("DROP TABLE poll_options"));
    }

    // Index so we can quickly look up all options for a given poll
    db.verifyIndex("pollOptionsPollID", "poll_options",
                   "(pollID)",
                   false, true);

    // Votes table — one row per vote cast
    const string votesTableSchema = R"(
        CREATE TABLE votes (
            voteID INTEGER PRIMARY KEY AUTOINCREMENT,
            pollID INTEGER NOT NULL,
            optionID INTEGER NOT NULL,
            createdAt INTEGER NOT NULL,
            FOREIGN KEY (pollID) REFERENCES polls(pollID),
            FOREIGN KEY (optionID) REFERENCES poll_options(optionID)
        )
    )";

    created = false;
    while (!db.verifyTable("votes", votesTableSchema, created)) {
        SASSERT(db.write("DROP TABLE votes"));
    }

    // Index for counting votes per option
    db.verifyIndex("votesOptionID", "votes",
                   "(optionID)",
                   false, true);
}
