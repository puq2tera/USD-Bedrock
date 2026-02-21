#include "Core.h"

#include "commands/chats/AddChatMember.h"
#include "commands/chats/CreateChat.h"
#include "commands/chats/DeleteChat.h"
#include "commands/chats/EditChat.h"
#include "commands/chats/EditChatMemberRole.h"
#include "commands/chats/GetChat.h"
#include "commands/chats/ListChatMembers.h"
#include "commands/chats/ListChats.h"
#include "commands/chats/RemoveChatMember.h"
#include "commands/chats/CreateChatMessage.h"
#include "commands/chats/DeleteChatMessage.h"
#include "commands/chats/EditChatMessage.h"
#include "commands/chats/GetChatMessages.h"
#include "commands/polls/CreatePoll.h"
#include "commands/polls/DeletePoll.h"
#include "commands/polls/DeletePollVotes.h"
#include "commands/polls/EditPoll.h"
#include "commands/polls/GetPoll.h"
#include "commands/polls/GetPollParticipation.h"
#include "commands/polls/ListPolls.h"
#include "commands/polls/SubmitPollTextResponse.h"
#include "commands/polls/SubmitPollVotes.h"
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
    if (SIEquals(baseCommand.request.methodLine, "CreateChat")) {
        return make_unique<CreateChat>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "GetChat")) {
        return make_unique<GetChat>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "ListChats")) {
        return make_unique<ListChats>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "EditChat")) {
        return make_unique<EditChat>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "DeleteChat")) {
        return make_unique<DeleteChat>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "AddChatMember")) {
        return make_unique<AddChatMember>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "ListChatMembers")) {
        return make_unique<ListChatMembers>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "EditChatMemberRole")) {
        return make_unique<EditChatMemberRole>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "RemoveChatMember")) {
        return make_unique<RemoveChatMember>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "CreateChatMessage")) {
        return make_unique<CreateChatMessage>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "GetChatMessages")) {
        return make_unique<GetChatMessages>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "EditChatMessage")) {
        return make_unique<EditChatMessage>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "DeleteChatMessage")) {
        return make_unique<DeleteChatMessage>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "CreatePoll")) {
        return make_unique<CreatePoll>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "GetPoll")) {
        return make_unique<GetPoll>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "GetPollParticipation")) {
        return make_unique<GetPollParticipation>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "ListPolls")) {
        return make_unique<ListPolls>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "EditPoll")) {
        return make_unique<EditPoll>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "DeletePoll")) {
        return make_unique<DeletePoll>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "SubmitPollVotes")) {
        return make_unique<SubmitPollVotes>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "DeletePollVotes")) {
        return make_unique<DeletePollVotes>(std::move(baseCommand), this);
    }
    if (SIEquals(baseCommand.request.methodLine, "SubmitPollTextResponse")) {
        return make_unique<SubmitPollTextResponse>(std::move(baseCommand), this);
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
