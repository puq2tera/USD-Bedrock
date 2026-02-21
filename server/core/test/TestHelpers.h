#pragma once

#include <atomic>
#include <libstuff/SData.h>
#include <libstuff/libstuff.h>
#include <test/lib/BedrockTester.h>
#include <atomic>

#ifndef CORE_TEST_PLUGIN_DIR
#    error "CORE_TEST_PLUGIN_DIR must be defined"
#endif

#ifndef CORE_TEST_BEDROCK_BIN
#    error "CORE_TEST_BEDROCK_BIN must be defined"
#endif

// Keep this helper surface limited to broadly reusable utilities that support
// many test suites. Per-command or one-off helpers should live in that suite.
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

<<<<<<< HEAD
    static void executeQuery(BedrockTester& tester, const string& sql) {
        SData req("Query");
        req["Query"] = SEndsWith(sql, ";") ? sql : (sql + ";");
        SData resp = executeSingle(tester, req);
        if (!SStartsWith(resp.methodLine, "200")) {
            STHROW("Test helper executeQuery failed: " + resp.methodLine + " SQL=" + sql);
        }
    }

=======
>>>>>>> origin/main
    static string createUserID(BedrockTester& tester,
                               const string& emailPrefix = "user",
                               const string& firstName = "Test",
                               const string& lastName = "User") {
        return createUser(tester, emailPrefix, firstName, lastName)["userID"];
    }

<<<<<<< HEAD
    static string createChatID(BedrockTester& tester,
                               const string& ownerUserID,
                               const string& title = "Test chat") {
        const string now = SToStr(STimeNow());
        executeQuery(
            tester,
            "INSERT INTO chats (title, createdAt, createdByUserID) VALUES (" +
                SQ(title) + ", " + now + ", " + ownerUserID + ")"
        );

        const string chatID = tester.readDB("SELECT chatID FROM chats ORDER BY chatID DESC LIMIT 1;");
        if (chatID.empty()) {
            STHROW("Test helper createChatID failed to read chatID");
        }

        addChatMember(tester, chatID, ownerUserID, "owner");
        return chatID;
    }

    static void addChatMember(BedrockTester& tester,
                              const string& chatID,
                              const string& userID,
                              const string& role = "member") {
        const string now = SToStr(STimeNow());
        executeQuery(
            tester,
            "INSERT OR IGNORE INTO chat_members (chatID, userID, role, joinedAt) VALUES (" +
                chatID + ", " + userID + ", " + SQ(role) + ", " + now + ")"
        );
    }

    static void addChatMember(BedrockTester& tester,
                              const string& chatID,
                              const string& actorUserID,
                              const string& targetUserID,
                              const string& role) {
        (void)actorUserID;
        addChatMember(tester, chatID, targetUserID, role);
    }

    static string createPollID(BedrockTester& tester,
                               const string& creatorUserID = "",
                               const string& question = "Test question?",
                               const string& options = "[\"Option A\",\"Option B\",\"Option C\"]",
                               const string& type = "single_choice",
                               bool allowChangeVote = false,
                               bool isAnonymous = false,
                               const string& expiresAt = "") {
        const string creatorID = creatorUserID.empty()
            ? createUserID(tester, "polls", "Poll", "Creator")
            : creatorUserID;

        const string chatID = createChatID(tester, creatorID, "Poll chat");
        return createPoll(
            tester,
            chatID,
            creatorID,
            question,
            options,
            type,
            allowChangeVote,
            isAnonymous,
            expiresAt
        )["pollID"];
    }

    static SData getPoll(BedrockTester& tester,
                         const string& pollID,
                         const string& requesterUserID = "") {
        const string requesterID = requesterUserID.empty()
            ? tester.readDB("SELECT creatorUserID FROM polls WHERE pollID = " + pollID + ";")
            : requesterUserID;

        ensureMemberForPoll(tester, pollID, requesterID);

        SData req("GetPoll");
        req["pollID"] = pollID;
        req["requesterUserID"] = requesterID;
        return executeSingle(tester, req);
    }

    static SData listPolls(BedrockTester& tester,
                           const string& chatID,
                           const string& requesterUserID,
                           bool includeClosed = true) {
        addChatMember(tester, chatID, requesterUserID, "member");

        SData req("ListPolls");
        req["chatID"] = chatID;
        req["requesterUserID"] = requesterUserID;
        req["includeClosed"] = includeClosed ? "true" : "false";
        return executeSingle(tester, req);
    }

    static STable firstOptionForPoll(BedrockTester& tester,
                                     const string& pollID,
                                     const string& requesterUserID = "") {
        const SData poll = getPoll(tester, pollID, requesterUserID);
=======
    static string createPollID(BedrockTester& tester,
                               const string& createdBy = "",
                               const string& question = "Test question?",
                               const string& options = "[\"Option A\",\"Option B\",\"Option C\"]") {
        return createPoll(tester, createdBy, question, options)["pollID"];
    }

    static STable firstOptionForPoll(BedrockTester& tester, const string& pollID) {
        const SData poll = getPoll(tester, pollID);
>>>>>>> origin/main
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
<<<<<<< HEAD
        return submitVotes(tester, pollID, {optionID}, userID);
    }

    static SData submitVotes(BedrockTester& tester,
                             const string& pollID,
                             const list<string>& optionIDs,
                             const string& userID) {
        ensureMemberForPoll(tester, pollID, userID);

        SData req("SubmitPollVotes");
        req["pollID"] = pollID;
        req["userID"] = userID;
        req["optionIDs"] = SComposeJSONArray(optionIDs);
        return executeSingle(tester, req);
    }

    static SData deleteVotes(BedrockTester& tester,
                             const string& pollID,
                             const string& userID) {
        ensureMemberForPoll(tester, pollID, userID);

        SData req("DeletePollVotes");
        req["pollID"] = pollID;
=======
        SData req("SubmitVote");
        req["pollID"] = pollID;
        req["optionID"] = optionID;
>>>>>>> origin/main
        req["userID"] = userID;
        return executeSingle(tester, req);
    }

<<<<<<< HEAD
    static SData submitTextResponse(BedrockTester& tester,
                                    const string& pollID,
                                    const string& userID,
                                    const string& textValue) {
        ensureMemberForPoll(tester, pollID, userID);

        SData req("SubmitPollTextResponse");
        req["pollID"] = pollID;
        req["userID"] = userID;
        req["textValue"] = textValue;
        return executeSingle(tester, req);
    }

    static string createChatMessageID(BedrockTester& tester,
                                      const string& chatID,
                                      const string& userID,
                                      const string& body) {
        return createMessage(tester, userID, chatID, body)["messageID"];
    }

=======
>>>>>>> origin/main
    static string createMessageID(BedrockTester& tester,
                                  const string& userID,
                                  const string& name,
                                  const string& message) {
<<<<<<< HEAD
        (void)name;
        const string chatID = createChatID(tester, userID, "Legacy message chat");
        return createMessage(tester, userID, chatID, message)["messageID"];
    }

private:
    static void ensureMemberForPoll(BedrockTester& tester,
                                    const string& pollID,
                                    const string& userID) {
        const string chatID = tester.readDB("SELECT chatID FROM polls WHERE pollID = " + pollID + ";");
        if (chatID.empty() || userID.empty()) {
            return;
        }
        addChatMember(tester, chatID, userID, "member");
    }

=======
        return createMessage(tester, userID, name, message)["messageID"];
    }

private:
>>>>>>> origin/main
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
<<<<<<< HEAD
                            const string& chatID,
                            const string& creatorUserID,
                            const string& question = "Test question?",
                            const string& options = "[\"Option A\",\"Option B\",\"Option C\"]",
                            const string& type = "single_choice",
                            bool allowChangeVote = false,
                            bool isAnonymous = false,
                            const string& expiresAt = "") {
        addChatMember(tester, chatID, creatorUserID, "owner");

        SData req("CreatePoll");
        req["chatID"] = chatID;
        req["creatorUserID"] = creatorUserID;
        req["question"] = question;
        req["type"] = type;
        req["allowChangeVote"] = allowChangeVote ? "true" : "false";
        req["isAnonymous"] = isAnonymous ? "true" : "false";
        if (type != "free_text") {
            req["options"] = options;
        }
        if (!expiresAt.empty()) {
            req["expiresAt"] = expiresAt;
        }
=======
                            const string& createdBy = "",
                            const string& question = "Test question?",
                            const string& options = "[\"Option A\",\"Option B\",\"Option C\"]") {
        const string creatorID = createdBy.empty() ? createUserID(tester, "polls", "Poll", "Creator") : createdBy;

        SData req("CreatePoll");
        req["createdBy"] = creatorID;
        req["question"] = question;
        req["options"] = options;
>>>>>>> origin/main

        SData resp = executeSingle(tester, req);
        if (!SStartsWith(resp.methodLine, "200 OK")) {
            STHROW("Test helper createPoll failed: " + resp.methodLine);
        }
        return resp;
    }

<<<<<<< HEAD
    static SData createMessage(BedrockTester& tester,
                               const string& userID,
                               const string& chatID,
                               const string& body) {
        addChatMember(tester, chatID, userID, "member");

        SData req("CreateChatMessage");
        req["chatID"] = chatID;
        req["userID"] = userID;
        req["body"] = body;
=======
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
>>>>>>> origin/main

        SData resp = executeSingle(tester, req);
        if (!SStartsWith(resp.methodLine, "200 OK")) {
            STHROW("Test helper createMessage failed: " + resp.methodLine);
        }
        return resp;
    }
};
