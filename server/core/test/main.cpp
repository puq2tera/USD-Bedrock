#include <iostream>
#include <test/lib/tpunit++.hpp>
#include <test/lib/BedrockTester.h>
#include <libstuff/libstuff.h>
#include <libstuff/SData.h>

#include "TestHelpers.h"
#include "tests/HelloWorldTest.h"
#include "tests/MessagesTest.h"
#include "tests/PollsTest.h"

void cleanup() {
    cout << "Cleaning up test database files...\n";
    if (system("rm -f coretest_*.db") == -1) {
        SWARN("system() failed.");
    }
    if (system("rm -f coretest_*.db-shm") == -1) {
        SWARN("system() failed.");
    }
    if (system("rm -f coretest_*.db-wal") == -1) {
        SWARN("system() failed.");
    }
    if (system("rm -f coretest_*.db-journal") == -1) {
        SWARN("system() failed.");
    }
    cout << "Cleanup complete.\n";
}

int main(int argc, char* argv[]) {
    SData args = SParseCommandLine(argc, argv);

    // Instantiate test fixtures
    HelloWorldTest helloWorldTest;
    MessagesTest messagesTest;
    PollsTest pollsTest;

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

