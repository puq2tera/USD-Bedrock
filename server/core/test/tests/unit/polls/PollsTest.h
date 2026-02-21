#pragma once

#include "../../../TestHelpers.h"
#include <iterator>

struct PollsTest : tpunit::TestFixture {
    PollsTest()
        : tpunit::TestFixture(
            "PollsTests",
            TEST(PollsTest::testCreatePollSuccess),
            TEST(PollsTest::testCreatePollValidationRules),
            TEST(PollsTest::testCreatePollRequiresChatMembership),
            TEST(PollsTest::testGetPollSuccess),
            TEST(PollsTest::testListPollsRespectsIncludeClosed),
            TEST(PollsTest::testListPollsRequiresChatMembership),
            TEST(PollsTest::testEditPollUpdatesFieldsAndOptions),
            TEST(PollsTest::testEditPollRequiresCreator),
            TEST(PollsTest::testEditPollMissingEditableFields),
            TEST(PollsTest::testEditPollRejectsOptionsForFreeText),
            TEST(PollsTest::testDeletePollSuccess),
            TEST(PollsTest::testDeletePollRequiresCreator),
            TEST(PollsTest::testDeletePollNotFound),
            TEST(PollsTest::testExpiryAutoCloseBehavior)
        ) { }

    string createPollInChat(BedrockTester& tester,
                            const string& chatID,
                            const string& creatorUserID,
                            const string& type,
                            const string& options = "[\"A\",\"B\",\"C\"]",
                            bool allowChangeVote = false,
                            bool isAnonymous = false,
                            const string& expiresAt = "") {
        SData req("CreatePoll");
        req["chatID"] = chatID;
        req["creatorUserID"] = creatorUserID;
        req["question"] = "Poll question?";
        req["type"] = type;
        req["allowChangeVote"] = allowChangeVote ? "true" : "false";
        req["isAnonymous"] = isAnonymous ? "true" : "false";
        if (type != "free_text") {
            req["options"] = options;
        }
        if (!expiresAt.empty()) {
            req["expiresAt"] = expiresAt;
        }

        SData resp = TestHelpers::executeSingle(tester, req);
        EXPECT_TRUE(SStartsWith(resp.methodLine, "200 OK"));
        return resp["pollID"];
    }

    void testCreatePollSuccess() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "pollcreate", "Poll", "Creator");
        const string chatID = TestHelpers::createChatID(tester, creatorID, "Poll Room");

        SData req("CreatePoll");
        req["chatID"] = chatID;
        req["creatorUserID"] = creatorID;
        req["question"] = "What should we ship?";
        req["type"] = "single_choice";
        req["options"] = "[\"A\",\"B\",\"C\"]";
        req["allowChangeVote"] = "true";
        req["isAnonymous"] = "false";

        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "200 OK"));
        ASSERT_FALSE(resp["pollID"].empty());
        ASSERT_EQUAL(resp["chatID"], chatID);
        ASSERT_EQUAL(resp["creatorUserID"], creatorID);
        ASSERT_EQUAL(resp["type"], "single_choice");
        ASSERT_EQUAL(resp["optionCount"], "3");
        ASSERT_EQUAL(resp["status"], "open");
    }

    void testCreatePollValidationRules() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "pollvalidate", "Poll", "Creator");
        const string chatID = TestHelpers::createChatID(tester, creatorID, "Poll Room");

        SData invalidTypeReq("CreatePoll");
        invalidTypeReq["chatID"] = chatID;
        invalidTypeReq["creatorUserID"] = creatorID;
        invalidTypeReq["question"] = "Bad type";
        invalidTypeReq["type"] = "ranked";
        invalidTypeReq["options"] = "[\"A\",\"B\"]";
        SData invalidTypeResp = TestHelpers::executeSingle(tester, invalidTypeReq);
        ASSERT_TRUE(SStartsWith(invalidTypeResp.methodLine, "400"));

        SData oneOptionReq("CreatePoll");
        oneOptionReq["chatID"] = chatID;
        oneOptionReq["creatorUserID"] = creatorID;
        oneOptionReq["question"] = "Not enough options";
        oneOptionReq["type"] = "single_choice";
        oneOptionReq["options"] = "[\"Only\"]";
        SData oneOptionResp = TestHelpers::executeSingle(tester, oneOptionReq);
        ASSERT_TRUE(SStartsWith(oneOptionResp.methodLine, "400"));

        SData duplicateOptionReq("CreatePoll");
        duplicateOptionReq["chatID"] = chatID;
        duplicateOptionReq["creatorUserID"] = creatorID;
        duplicateOptionReq["question"] = "Duplicate options";
        duplicateOptionReq["type"] = "single_choice";
        duplicateOptionReq["options"] = "[\"A\",\"A\"]";
        SData duplicateOptionResp = TestHelpers::executeSingle(tester, duplicateOptionReq);
        ASSERT_TRUE(SStartsWith(duplicateOptionResp.methodLine, "400"));

        SData freeTextWithOptionsReq("CreatePoll");
        freeTextWithOptionsReq["chatID"] = chatID;
        freeTextWithOptionsReq["creatorUserID"] = creatorID;
        freeTextWithOptionsReq["question"] = "Free text";
        freeTextWithOptionsReq["type"] = "free_text";
        freeTextWithOptionsReq["options"] = "[\"Ignored\",\"Also ignored\"]";
        SData freeTextWithOptionsResp = TestHelpers::executeSingle(tester, freeTextWithOptionsReq);
        ASSERT_TRUE(SStartsWith(freeTextWithOptionsResp.methodLine, "400"));
    }

    void testCreatePollRequiresChatMembership() {
        BedrockTester tester = TestHelpers::createTester();
        const string ownerID = TestHelpers::createUserID(tester, "poll-owner", "Poll", "Owner");
        const string outsiderID = TestHelpers::createUserID(tester, "poll-outsider", "Poll", "Outsider");
        const string chatID = TestHelpers::createChatID(tester, ownerID, "Membership chat");

        SData req("CreatePoll");
        req["chatID"] = chatID;
        req["creatorUserID"] = outsiderID;
        req["question"] = "Can outsider create?";
        req["type"] = "single_choice";
        req["options"] = "[\"A\",\"B\"]";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "409"));
    }

    void testGetPollSuccess() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "pollget", "Poll", "Getter");
        const string pollID = TestHelpers::createPollID(tester, creatorID);

        SData resp = TestHelpers::getPoll(tester, pollID, creatorID);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "200 OK"));
        ASSERT_EQUAL(resp["pollID"], pollID);
        ASSERT_EQUAL(resp["creatorUserID"], creatorID);
        ASSERT_EQUAL(resp["type"], "single_choice");
        ASSERT_EQUAL(resp["optionCount"], "3");
        ASSERT_EQUAL(resp["totalVotes"], "0");
    }

    void testListPollsRespectsIncludeClosed() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "poll-list", "Poll", "Lister");
        const string chatID = TestHelpers::createChatID(tester, creatorID, "List polls");

        const string openPollID = createPollInChat(tester, chatID, creatorID, "single_choice", "[\"A\",\"B\"]");
        const string closedPollID = createPollInChat(tester, chatID, creatorID, "single_choice", "[\"C\",\"D\"]");

        SData closeReq("EditPoll");
        closeReq["pollID"] = closedPollID;
        closeReq["actorUserID"] = creatorID;
        closeReq["status"] = "closed";
        SData closeResp = TestHelpers::executeSingle(tester, closeReq);
        ASSERT_TRUE(SStartsWith(closeResp.methodLine, "200 OK"));

        SData openOnlyReq("ListPolls");
        openOnlyReq["chatID"] = chatID;
        openOnlyReq["requesterUserID"] = creatorID;
        openOnlyReq["includeClosed"] = "false";
        SData openOnlyResp = TestHelpers::executeSingle(tester, openOnlyReq);
        ASSERT_TRUE(SStartsWith(openOnlyResp.methodLine, "200 OK"));

        list<string> openOnlyPolls = SParseJSONArray(openOnlyResp["polls"]);
        ASSERT_EQUAL(openOnlyPolls.size(), static_cast<size_t>(1));
        ASSERT_EQUAL(SParseJSONObject(openOnlyPolls.front()).at("pollID"), openPollID);

        SData includeClosedReq("ListPolls");
        includeClosedReq["chatID"] = chatID;
        includeClosedReq["requesterUserID"] = creatorID;
        includeClosedReq["includeClosed"] = "true";
        SData includeClosedResp = TestHelpers::executeSingle(tester, includeClosedReq);
        ASSERT_TRUE(SStartsWith(includeClosedResp.methodLine, "200 OK"));

        list<string> includeClosedPolls = SParseJSONArray(includeClosedResp["polls"]);
        ASSERT_EQUAL(includeClosedPolls.size(), static_cast<size_t>(2));
    }

    void testListPollsRequiresChatMembership() {
        BedrockTester tester = TestHelpers::createTester();
        const string ownerID = TestHelpers::createUserID(tester, "poll-list-owner", "Poll", "Owner");
        const string outsiderID = TestHelpers::createUserID(tester, "poll-list-outsider", "Poll", "Outsider");
        const string chatID = TestHelpers::createChatID(tester, ownerID, "List polls");
        (void)createPollInChat(tester, chatID, ownerID, "single_choice", "[\"A\",\"B\"]");

        SData req("ListPolls");
        req["chatID"] = chatID;
        req["requesterUserID"] = outsiderID;
        req["includeClosed"] = "true";
        const SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "409"));
    }

    void testEditPollUpdatesFieldsAndOptions() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "poll-edit", "Poll", "Editor");
        const string chatID = TestHelpers::createChatID(tester, creatorID, "Edit poll");
        const string pollID = createPollInChat(tester, chatID, creatorID, "single_choice", "[\"A\",\"B\"]");

        SData editReq("EditPoll");
        editReq["pollID"] = pollID;
        editReq["actorUserID"] = creatorID;
        editReq["question"] = "Updated question";
        editReq["allowChangeVote"] = "true";
        editReq["isAnonymous"] = "true";
        editReq["options"] = "[\"One\",\"Two\",\"Three\"]";
        SData editResp = TestHelpers::executeSingle(tester, editReq);

        ASSERT_TRUE(SStartsWith(editResp.methodLine, "200 OK"));
        ASSERT_EQUAL(editResp["result"], "updated");
        ASSERT_EQUAL(editResp["optionCount"], "3");

        SData getResp = TestHelpers::getPoll(tester, pollID, creatorID);
        ASSERT_TRUE(SStartsWith(getResp.methodLine, "200 OK"));
        ASSERT_EQUAL(getResp["question"], "Updated question");
        ASSERT_EQUAL(getResp["allowChangeVote"], "true");
        ASSERT_EQUAL(getResp["isAnonymous"], "true");
        ASSERT_EQUAL(getResp["optionCount"], "3");

        list<string> options = SParseJSONArray(getResp["options"]);
        ASSERT_EQUAL(options.size(), static_cast<size_t>(3));
        ASSERT_EQUAL(SParseJSONObject(options.front()).at("label"), "One");
    }

    void testEditPollRequiresCreator() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "poll-owner", "Poll", "Owner");
        const string memberID = TestHelpers::createUserID(tester, "poll-member", "Poll", "Member");
        const string chatID = TestHelpers::createChatID(tester, creatorID, "Edit poll");
        TestHelpers::addChatMember(tester, chatID, creatorID, memberID, "member");

        const string pollID = createPollInChat(tester, chatID, creatorID, "single_choice", "[\"A\",\"B\"]");

        SData editReq("EditPoll");
        editReq["pollID"] = pollID;
        editReq["actorUserID"] = memberID;
        editReq["question"] = "Not allowed";
        SData editResp = TestHelpers::executeSingle(tester, editReq);

        ASSERT_TRUE(SStartsWith(editResp.methodLine, "409"));
    }

    void testEditPollMissingEditableFields() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "poll-edit-empty-owner", "Poll", "Owner");
        const string chatID = TestHelpers::createChatID(tester, creatorID, "Edit poll");
        const string pollID = createPollInChat(tester, chatID, creatorID, "single_choice", "[\"A\",\"B\"]");

        SData editReq("EditPoll");
        editReq["pollID"] = pollID;
        editReq["actorUserID"] = creatorID;
        const SData editResp = TestHelpers::executeSingle(tester, editReq);

        ASSERT_TRUE(SStartsWith(editResp.methodLine, "400"));
    }

    void testEditPollRejectsOptionsForFreeText() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "poll-edit-free-text-owner", "Poll", "Owner");
        const string chatID = TestHelpers::createChatID(tester, creatorID, "Edit poll");
        const string pollID = createPollInChat(tester, chatID, creatorID, "free_text");

        SData editReq("EditPoll");
        editReq["pollID"] = pollID;
        editReq["actorUserID"] = creatorID;
        editReq["options"] = "[\"A\",\"B\"]";
        const SData editResp = TestHelpers::executeSingle(tester, editReq);

        ASSERT_TRUE(SStartsWith(editResp.methodLine, "400"));
    }

    void testDeletePollSuccess() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "poll-delete", "Poll", "Owner");
        const string chatID = TestHelpers::createChatID(tester, creatorID, "Delete poll");
        const string pollID = createPollInChat(tester, chatID, creatorID, "single_choice", "[\"A\",\"B\"]");

        SData deleteReq("DeletePoll");
        deleteReq["pollID"] = pollID;
        deleteReq["actorUserID"] = creatorID;
        SData deleteResp = TestHelpers::executeSingle(tester, deleteReq);

        ASSERT_TRUE(SStartsWith(deleteResp.methodLine, "200 OK"));
        ASSERT_EQUAL(deleteResp["result"], "deleted");

        SData getReq("GetPoll");
        getReq["pollID"] = pollID;
        getReq["requesterUserID"] = creatorID;
        SData getResp = TestHelpers::executeSingle(tester, getReq);
        ASSERT_TRUE(SStartsWith(getResp.methodLine, "404"));
    }

    void testDeletePollRequiresCreator() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "poll-owner", "Poll", "Owner");
        const string memberID = TestHelpers::createUserID(tester, "poll-member", "Poll", "Member");
        const string chatID = TestHelpers::createChatID(tester, creatorID, "Delete poll");
        TestHelpers::addChatMember(tester, chatID, creatorID, memberID, "member");

        const string pollID = createPollInChat(tester, chatID, creatorID, "single_choice", "[\"A\",\"B\"]");

        SData deleteReq("DeletePoll");
        deleteReq["pollID"] = pollID;
        deleteReq["actorUserID"] = memberID;
        SData deleteResp = TestHelpers::executeSingle(tester, deleteReq);

        ASSERT_TRUE(SStartsWith(deleteResp.methodLine, "409"));
    }

    void testDeletePollNotFound() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "poll-delete-owner", "Poll", "Owner");

        SData req("DeletePoll");
        req["pollID"] = "99999";
        req["actorUserID"] = creatorID;
        const SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "404"));
    }

    void testExpiryAutoCloseBehavior() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "expiry-owner", "Expiry", "Owner");
        const string chatID = TestHelpers::createChatID(tester, creatorID, "Expiry chat");

        const string expiredAt = SToStr(STimeNow() - 1);
        const string pollID = createPollInChat(
            tester,
            chatID,
            creatorID,
            "single_choice",
            "[\"Now\",\"Later\"]",
            false,
            false,
            expiredAt
        );

        SData getResp = TestHelpers::getPoll(tester, pollID, creatorID);
        ASSERT_TRUE(SStartsWith(getResp.methodLine, "200 OK"));
        ASSERT_EQUAL(getResp["status"], "closed");
        ASSERT_FALSE(getResp["closedAt"].empty());

        const STable firstOption = TestHelpers::firstOptionForPoll(tester, pollID, creatorID);
        const string voterID = TestHelpers::createUserID(tester, "expiry-voter", "Expiry", "Voter");
        SData voteResp = TestHelpers::submitVote(tester, pollID, firstOption.at("optionID"), voterID);
        ASSERT_TRUE(SStartsWith(voteResp.methodLine, "409"));
    }
};
