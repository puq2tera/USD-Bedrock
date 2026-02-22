#pragma once

#include "../../../TestHelpers.h"

struct PollTextResponsesTest : tpunit::TestFixture {
    PollTextResponsesTest()
        : tpunit::TestFixture(
            "PollTextResponsesTests",
            TEST(PollTextResponsesTest::testSubmitTextResponseSuccessAndGet),
            TEST(PollTextResponsesTest::testSubmitTextResponseRequiresChatMembership),
            TEST(PollTextResponsesTest::testSubmitTextResponseWrongTypeForChoicePoll),
            TEST(PollTextResponsesTest::testSubmitTextResponseRejectsBlankText),
            TEST(PollTextResponsesTest::testSubmitTextResponseRespectsAllowChangeVote),
            TEST(PollTextResponsesTest::testSubmitTextResponseRejectsClosedPoll),
            TEST(PollTextResponsesTest::testAnonymousPollHidesResponderIdentity)
        ) { }

    string createFreeTextPoll(BedrockTester& tester,
                              const string& creatorUserID,
                              bool allowChangeVote = false,
                              bool isAnonymous = false) {
        const string chatID = TestHelpers::createChatID(tester, creatorUserID, "Poll text responses chat");

        SData req("CreatePoll");
        req["chatID"] = chatID;
        req["creatorUserID"] = creatorUserID;
        req["question"] = "Text response?";
        req["type"] = "free_text";
        req["allowChangeVote"] = allowChangeVote ? "true" : "false";
        req["isAnonymous"] = isAnonymous ? "true" : "false";
        const SData resp = TestHelpers::executeSingle(tester, req);

        EXPECT_TRUE(SStartsWith(resp.methodLine, "200 OK"));
        return resp["pollID"];
    }

    void testSubmitTextResponseSuccessAndGet() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "text-owner", "Text", "Owner");
        const string responderID = TestHelpers::createUserID(tester, "text-responder", "Text", "Responder");
        const string pollID = createFreeTextPoll(tester, creatorID, false, false);

        const SData submitResp = TestHelpers::submitTextResponse(tester, pollID, responderID, "First response");
        ASSERT_TRUE(SStartsWith(submitResp.methodLine, "200 OK"));
        ASSERT_EQUAL(submitResp["replaced"], "false");
        ASSERT_EQUAL(submitResp["textValue"], "First response");

        const SData getResp = TestHelpers::getPoll(tester, pollID, creatorID);
        ASSERT_TRUE(SStartsWith(getResp.methodLine, "200 OK"));
        ASSERT_EQUAL(getResp["responseCount"], "1");

        list<string> responses = SParseJSONArray(getResp["responses"]);
        ASSERT_EQUAL(responses.size(), static_cast<size_t>(1));
        const STable row = SParseJSONObject(responses.front());
        ASSERT_EQUAL(row.at("userID"), responderID);
        ASSERT_EQUAL(row.at("textValue"), "First response");
    }

    void testSubmitTextResponseRequiresChatMembership() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "text-owner", "Text", "Owner");
        const string outsiderID = TestHelpers::createUserID(tester, "text-outsider", "Text", "Outsider");
        const string pollID = createFreeTextPoll(tester, creatorID);

        SData req("SubmitPollTextResponse");
        req["pollID"] = pollID;
        req["userID"] = outsiderID;
        req["textValue"] = "Not a member";
        const SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "409"));
    }

    void testSubmitTextResponseWrongTypeForChoicePoll() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "text-choice-owner", "Text", "Owner");
        const string pollID = TestHelpers::createPollID(tester, creatorID, "Choice poll", "[\"A\",\"B\"]", "single_choice");

        SData req("SubmitPollTextResponse");
        req["pollID"] = pollID;
        req["userID"] = creatorID;
        req["textValue"] = "Wrong type";
        const SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testSubmitTextResponseRejectsBlankText() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "text-blank-owner", "Text", "Owner");
        const string pollID = createFreeTextPoll(tester, creatorID);

        SData req("SubmitPollTextResponse");
        req["pollID"] = pollID;
        req["userID"] = creatorID;
        req["textValue"] = "   ";
        const SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testSubmitTextResponseRespectsAllowChangeVote() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "text-change-owner", "Text", "Owner");
        const string responderID = TestHelpers::createUserID(tester, "text-change-responder", "Text", "Responder");

        const string noChangePollID = createFreeTextPoll(tester, creatorID, false, false);
        const SData firstNoChange = TestHelpers::submitTextResponse(tester, noChangePollID, responderID, "First");
        ASSERT_TRUE(SStartsWith(firstNoChange.methodLine, "200 OK"));

        const SData secondNoChange = TestHelpers::submitTextResponse(tester, noChangePollID, responderID, "Second");
        ASSERT_TRUE(SStartsWith(secondNoChange.methodLine, "409"));

        const string allowChangePollID = createFreeTextPoll(tester, creatorID, true, false);
        const SData firstAllow = TestHelpers::submitTextResponse(tester, allowChangePollID, responderID, "One");
        ASSERT_TRUE(SStartsWith(firstAllow.methodLine, "200 OK"));

        const SData secondAllow = TestHelpers::submitTextResponse(tester, allowChangePollID, responderID, "Two");
        ASSERT_TRUE(SStartsWith(secondAllow.methodLine, "200 OK"));
        ASSERT_EQUAL(secondAllow["replaced"], "true");

        const SData allowPollResp = TestHelpers::getPoll(tester, allowChangePollID, creatorID);
        ASSERT_TRUE(SStartsWith(allowPollResp.methodLine, "200 OK"));
        ASSERT_EQUAL(allowPollResp["responseCount"], "1");
        list<string> responses = SParseJSONArray(allowPollResp["responses"]);
        ASSERT_EQUAL(responses.size(), static_cast<size_t>(1));
        ASSERT_EQUAL(SParseJSONObject(responses.front()).at("textValue"), "Two");
    }

    void testSubmitTextResponseRejectsClosedPoll() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "text-closed-owner", "Text", "Owner");
        const string pollID = createFreeTextPoll(tester, creatorID);

        SData closeReq("EditPoll");
        closeReq["pollID"] = pollID;
        closeReq["actorUserID"] = creatorID;
        closeReq["status"] = "closed";
        const SData closeResp = TestHelpers::executeSingle(tester, closeReq);
        ASSERT_TRUE(SStartsWith(closeResp.methodLine, "200 OK"));

        SData submitReq("SubmitPollTextResponse");
        submitReq["pollID"] = pollID;
        submitReq["userID"] = creatorID;
        submitReq["textValue"] = "Late response";
        const SData submitResp = TestHelpers::executeSingle(tester, submitReq);

        ASSERT_TRUE(SStartsWith(submitResp.methodLine, "409"));
    }

    void testAnonymousPollHidesResponderIdentity() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "text-anon-owner", "Text", "Owner");
        const string responderID = TestHelpers::createUserID(tester, "text-anon-user", "Text", "Responder");
        const string pollID = createFreeTextPoll(tester, creatorID, true, true);

        const SData submitResp = TestHelpers::submitTextResponse(tester, pollID, responderID, "Anonymous answer");
        ASSERT_TRUE(SStartsWith(submitResp.methodLine, "200 OK"));

        const SData getResp = TestHelpers::getPoll(tester, pollID, creatorID);
        ASSERT_TRUE(SStartsWith(getResp.methodLine, "200 OK"));
        ASSERT_EQUAL(getResp["isAnonymous"], "true");

        list<string> responses = SParseJSONArray(getResp["responses"]);
        ASSERT_EQUAL(responses.size(), static_cast<size_t>(1));
        const STable row = SParseJSONObject(responses.front());
        ASSERT_FALSE(row.count("userID"));
        ASSERT_EQUAL(row.at("textValue"), "Anonymous answer");
    }
};
