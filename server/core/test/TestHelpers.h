#pragma once

#include <libstuff/libstuff.h>
#include <test/lib/BedrockTester.h>
#include <atomic>

#ifndef CORE_TEST_PLUGIN_DIR
#    error "CORE_TEST_PLUGIN_DIR must be defined"
#endif

#ifndef CORE_TEST_BEDROCK_BIN
#    error "CORE_TEST_BEDROCK_BIN must be defined"
#endif

class TestHelpers {
public:
    static BedrockTester createTester() {
        const string corePluginPath = string(CORE_TEST_PLUGIN_DIR) + "/Core.so";
        map<string, string> args = {
            {"-plugins", "DB," + corePluginPath},
            {"-db", BedrockTester::getTempFileName("coretest")}
        };

        return {args, {}, 0, 0, 0, true, CORE_TEST_BEDROCK_BIN};
    }

    static SData executeSingle(BedrockTester& tester, const SData& request) {
        vector<SData> responses = tester.executeWaitMultipleData({request}, 1);
        if (responses.empty()) {
            STHROW("Test helper executeSingle received no response");
        }
        return responses.front();
    }

    static string createUserID(BedrockTester& tester,
                               const string& emailPrefix = "user",
                               const string& firstName = "Test",
                               const string& lastName = "User") {
        return createUser(tester, emailPrefix, firstName, lastName)["userID"];
    }

    static string createPollID(BedrockTester& tester,
                               const string& createdBy = "",
                               const string& question = "Test question?",
                               const string& options = "[\"Option A\",\"Option B\",\"Option C\"]") {
        return createPoll(tester, createdBy, question, options)["pollID"];
    }

    static STable firstOptionForPoll(BedrockTester& tester, const string& pollID) {
        const SData poll = getPoll(tester, pollID);
        list<string> options = SParseJSONArray(poll["options"]);
        if (options.empty()) {
            STHROW("Test helper firstOptionForPoll found no options");
        }
        return SParseJSONObject(options.front());
    }

    static SData submitVote(BedrockTester& tester,
                            const string& pollID,
                            const string& optionID,
                            const string& userID) {
        SData req("SubmitVote");
        req["pollID"] = pollID;
        req["optionID"] = optionID;
        req["userID"] = userID;
        return executeSingle(tester, req);
    }

    static string createMessageID(BedrockTester& tester,
                                  const string& userID,
                                  const string& name,
                                  const string& message) {
        return createMessage(tester, userID, name, message)["messageID"];
    }

private:
    static string uniqueEmail(const string& prefix = "user") {
        static atomic<uint64_t> counter {0};
        return prefix + "-" + SToStr(STimeNow()) + "-" + SToStr(++counter) + "@example.com";
    }

    static SData createUser(BedrockTester& tester,
                            const string& emailPrefix = "user",
                            const string& firstName = "Test",
                            const string& lastName = "User") {
        SData req("CreateUser");
        req["email"] = uniqueEmail(emailPrefix);
        req["firstName"] = firstName;
        req["lastName"] = lastName;

        SData resp = executeSingle(tester, req);
        if (!SStartsWith(resp.methodLine, "200 OK")) {
            STHROW("Test helper createUser failed: " + resp.methodLine);
        }
        return resp;
    }

    static SData createPoll(BedrockTester& tester,
                            const string& createdBy = "",
                            const string& question = "Test question?",
                            const string& options = "[\"Option A\",\"Option B\",\"Option C\"]") {
        const string creatorID = createdBy.empty() ? createUserID(tester, "polls", "Poll", "Creator") : createdBy;

        SData req("CreatePoll");
        req["createdBy"] = creatorID;
        req["question"] = question;
        req["options"] = options;

        SData resp = executeSingle(tester, req);
        if (!SStartsWith(resp.methodLine, "200 OK")) {
            STHROW("Test helper createPoll failed: " + resp.methodLine);
        }
        return resp;
    }

    static SData getPoll(BedrockTester& tester, const string& pollID) {
        SData req("GetPoll");
        req["pollID"] = pollID;
        return executeSingle(tester, req);
    }

    static SData createMessage(BedrockTester& tester,
                               const string& userID,
                               const string& name,
                               const string& message) {
        SData req("CreateMessage");
        req["userID"] = userID;
        req["name"] = name;
        req["message"] = message;

        SData resp = executeSingle(tester, req);
        if (!SStartsWith(resp.methodLine, "200 OK")) {
            STHROW("Test helper createMessage failed: " + resp.methodLine);
        }
        return resp;
    }
};
