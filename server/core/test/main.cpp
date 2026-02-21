#include <iostream>
#include <test/lib/tpunit++.hpp>
#include <test/lib/BedrockTester.h>
#include <libstuff/libstuff.h>
#include <libstuff/SData.h>

#include "TestHelpers.h"
#include "tests/unit/chats/ChatMembersTest.h"
#include "tests/unit/chats/ChatsTest.h"
#include "tests/unit/chats/ChatMessagesTest.h"
#include "tests/unit/polls/PollFeaturesTest.h"
#include "tests/unit/polls/PollsTest.h"
#include "tests/unit/polls/PollTextResponsesTest.h"
#include "tests/unit/polls/PollVotesTest.h"
#include "tests/unit/system/HelloWorldTest.h"
#include "tests/unit/users/UsersTest.h"

void cleanup() {
    cout << "Cleaning up test database files...\n";
    for (const string& suffix : {"db", "db-shm", "db-wal", "db-journal"}) {
        const string command = "rm -f coretest_*." + suffix;
        if (system(command.c_str()) == -1) {
            SWARN("system() failed for cleanup command: " << command);
        }
    }
    cout << "Cleanup complete.\n";
}

int main(int argc, char* argv[]) {
    SData args = SParseCommandLine(argc, argv);

    HelloWorldTest helloWorldTest;
    ChatsTest chatsTest;
    ChatMembersTest chatMembersTest;
    ChatMessagesTest chatMessagesTest;
    PollFeaturesTest pollFeaturesTest;
    PollsTest pollsTest;
    PollVotesTest pollVotesTest;
    PollTextResponsesTest pollTextResponsesTest;
    UsersTest usersTest;

    set<string> include;
    set<string> exclude;
    list<string> before;
    list<string> after;

    SLogLevel(LOG_INFO);
    if (args.isSet("-v")) {
        SLogLevel(LOG_DEBUG);
    }
    if (args.isSet("-q")) {
        SLogLevel(LOG_WARNING);
    }

    if (args.isSet("-only")) {
        list<string> includeList = SParseList(args["-only"]);
        for (const string& name : includeList) {
            include.insert(name);
        }
    }

    if (args.isSet("-except")) {
        list<string> excludeList = SParseList(args["-except"]);
        for (const string& name : excludeList) {
            exclude.insert(name);
        }
    }

    int retval = 0;
    try {
        retval = (int) tpunit::Tests::run(include, exclude, before, after, 1, []() {
            SLogSetThreadName("");
            SLogSetThreadPrefix("");
        });
    } catch (...) {
        cout << "Unhandled exception running tests!\n";
        cleanup();
        return 1;
    }

    cleanup();
    return retval;
}
