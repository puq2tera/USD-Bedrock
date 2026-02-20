#include "Core.h"

#include "commands/messages/CreateMessage.h"
#include "commands/messages/GetMessages.h"
#include "commands/polls/CreatePoll.h"
#include "commands/polls/DeletePoll.h"
#include "commands/polls/EditPoll.h"
#include "commands/polls/GetPoll.h"
#include "commands/polls/SubmitVote.h"
#include "commands/system/HelloWorld.h"
#include "commands/users/CreateUser.h"
#include "commands/users/DeleteUser.h"
#include "commands/users/EditUser.h"
#include "commands/users/GetUser.h"
#include "tables/Tables.h"

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
    if (SIEquals(baseCommand.request.methodLine, "SubmitVote")) {
        return make_unique<SubmitVote>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "EditPoll")) {
        return make_unique<EditPoll>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "DeletePoll")) {
        return make_unique<DeletePoll>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "CreateUser")) {
        return make_unique<CreateUser>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "GetUser")) {
        return make_unique<GetUser>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "EditUser")) {
        return make_unique<EditUser>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "DeleteUser")) {
        return make_unique<DeleteUser>(std::move(baseCommand), this);
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
    Tables::verifyAll(db);
}
