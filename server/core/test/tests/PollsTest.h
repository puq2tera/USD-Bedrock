#pragma once

#include "../TestHelpers.h"
#include <libstuff/SData.h>

struct PollsTest : tpunit::TestFixture {
    PollsTest()
        : tpunit::TestFixture(
            "PollsTests",

            TEST(PollsTest::testCreatePollSuccess),
            TEST(PollsTest::testCreatePollMissingQuestion),
            TEST(PollsTest::testCreatePollMissingOptions),
            TEST(PollsTest::testCreatePollMissingCreatedBy),
            TEST(PollsTest::testCreatePollInvalidCreatedBy),
            TEST(PollsTest::testCreatePollTooFewOptions),
            TEST(PollsTest::testCreatePollTooManyOptions),
            TEST(PollsTest::testCreatePollEmptyOptionText),
            TEST(PollsTest::testCreatePollDuplicateOptions),
            TEST(PollsTest::testCreatePollInvalidOptionsJSON),

            TEST(PollsTest::testGetPollSuccess),
            TEST(PollsTest::testGetPollNotFound),
            TEST(PollsTest::testGetPollInvalidID),
            TEST(PollsTest::testGetPollMissingID),

            TEST(PollsTest::testSubmitVoteSuccess),
            TEST(PollsTest::testSubmitVoteWrongOption),
            TEST(PollsTest::testSubmitVoteInvalidPoll),
            TEST(PollsTest::testSubmitVoteInvalidOptionID),
            TEST(PollsTest::testSubmitVoteInvalidUserID),
            TEST(PollsTest::testSubmitVoteInvalidUserIDFormat),
            TEST(PollsTest::testSubmitVoteMissingPollID),
            TEST(PollsTest::testSubmitVoteMissingOptionID),
            TEST(PollsTest::testSubmitVoteMissingUserID),
            TEST(PollsTest::testSubmitVoteDuplicateUserOnPoll),

            TEST(PollsTest::testGetPollWithVoteCounts),

            TEST(PollsTest::testEditPollQuestion),
            TEST(PollsTest::testEditPollOptions),
            TEST(PollsTest::testEditPollQuestionAndOptions),
            TEST(PollsTest::testEditPollInvalidID),
            TEST(PollsTest::testEditPollInvalidOptionsJSON),
            TEST(PollsTest::testEditPollTooFewOptions),
            TEST(PollsTest::testEditPollDuplicateOptions),
            TEST(PollsTest::testEditPollNoFields),
            TEST(PollsTest::testEditPollNotFound),

            TEST(PollsTest::testDeletePollSuccess),
            TEST(PollsTest::testDeletePollNotFound),
            TEST(PollsTest::testDeletePollInvalidID)
        ) { }

    SData getPoll(BedrockTester& tester, const string& pollID) {
        SData req("GetPoll");
        req["pollID"] = pollID;
        return TestHelpers::executeSingle(tester, req);
    }

    void testCreatePollSuccess() {
        BedrockTester tester = TestHelpers::createTester();
        const string createdBy = TestHelpers::createUserID(tester, "polls", "Poll", "User");

        SData req("CreatePoll");
        req["createdBy"] = createdBy;
        req["question"] = "What is your favorite color?";
        req["options"] = "[\"Red\",\"Blue\",\"Green\"]";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "200 OK"));
        ASSERT_FALSE(resp["pollID"].empty());
        ASSERT_EQUAL(resp["question"], "What is your favorite color?");
        ASSERT_EQUAL(resp["createdBy"], createdBy);
        ASSERT_EQUAL(resp["optionCount"], "3");
        ASSERT_FALSE(resp["createdAt"].empty());
    }

    void testCreatePollMissingQuestion() {
        BedrockTester tester = TestHelpers::createTester();
        const string createdBy = TestHelpers::createUserID(tester, "polls");

        SData req("CreatePoll");
        req["createdBy"] = createdBy;
        req["options"] = "[\"A\",\"B\"]";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testCreatePollMissingOptions() {
        BedrockTester tester = TestHelpers::createTester();
        const string createdBy = TestHelpers::createUserID(tester, "polls");

        SData req("CreatePoll");
        req["createdBy"] = createdBy;
        req["question"] = "Missing options?";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testCreatePollMissingCreatedBy() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("CreatePoll");
        req["question"] = "Missing creator?";
        req["options"] = "[\"A\",\"B\"]";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
        ASSERT_EQUAL(resp["errorCode"], "MISSING_PARAMETER");
    }

    void testCreatePollInvalidCreatedBy() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("CreatePoll");
        req["createdBy"] = "99999";
        req["question"] = "Missing creator?";
        req["options"] = "[\"A\",\"B\"]";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "404"));
    }

    void testCreatePollTooFewOptions() {
        BedrockTester tester = TestHelpers::createTester();
        const string createdBy = TestHelpers::createUserID(tester, "polls");

        SData req("CreatePoll");
        req["createdBy"] = createdBy;
        req["question"] = "Only one option?";
        req["options"] = "[\"Lonely\"]";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testCreatePollTooManyOptions() {
        BedrockTester tester = TestHelpers::createTester();
        const string createdBy = TestHelpers::createUserID(tester, "polls");

        SData req("CreatePoll");
        req["createdBy"] = createdBy;
        req["question"] = "Too many?";
        req["options"] = "[\"1\",\"2\",\"3\",\"4\",\"5\",\"6\",\"7\",\"8\",\"9\",\"10\","
                         "\"11\",\"12\",\"13\",\"14\",\"15\",\"16\",\"17\",\"18\",\"19\",\"20\",\"21\"]";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testCreatePollEmptyOptionText() {
        BedrockTester tester = TestHelpers::createTester();
        const string createdBy = TestHelpers::createUserID(tester, "polls");

        SData req("CreatePoll");
        req["createdBy"] = createdBy;
        req["question"] = "Empty option?";
        req["options"] = "[\"Good\",\"\",\"Bad\"]";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testCreatePollDuplicateOptions() {
        BedrockTester tester = TestHelpers::createTester();
        const string createdBy = TestHelpers::createUserID(tester, "polls");

        SData req("CreatePoll");
        req["createdBy"] = createdBy;
        req["question"] = "Duplicates?";
        req["options"] = "[\"Apple\",\"Banana\",\"Apple\"]";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testCreatePollInvalidOptionsJSON() {
        BedrockTester tester = TestHelpers::createTester();
        const string createdBy = TestHelpers::createUserID(tester, "polls");

        SData req("CreatePoll");
        req["createdBy"] = createdBy;
        req["question"] = "Bad json?";
        req["options"] = "not-json";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testGetPollSuccess() {
        BedrockTester tester = TestHelpers::createTester();
        const string createdBy = TestHelpers::createUserID(tester, "polls");
        const string pollID = TestHelpers::createPollID(tester, createdBy);

        SData req("GetPoll");
        req["pollID"] = pollID;
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "200 OK"));
        ASSERT_EQUAL(resp["pollID"], pollID);
        ASSERT_EQUAL(resp["createdBy"], createdBy);
        ASSERT_EQUAL(resp["question"], "Test question?");
        ASSERT_EQUAL(resp["optionCount"], "3");
        ASSERT_EQUAL(resp["totalVotes"], "0");
        ASSERT_FALSE(resp["options"].empty());
    }

    void testGetPollNotFound() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("GetPoll");
        req["pollID"] = "99999";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "404"));
    }

    void testGetPollInvalidID() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("GetPoll");
        req["pollID"] = "0";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testGetPollMissingID() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("GetPoll");
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testSubmitVoteSuccess() {
        BedrockTester tester = TestHelpers::createTester();
        const string pollID = TestHelpers::createPollID(tester);
        const string voterID = TestHelpers::createUserID(tester, "vote", "Vote", "User");
        const STable firstOption = TestHelpers::firstOptionForPoll(tester, pollID);
        const string optionID = firstOption.at("optionID");

        SData voteResp = TestHelpers::submitVote(tester, pollID, optionID, voterID);

        ASSERT_TRUE(SStartsWith(voteResp.methodLine, "200 OK"));
        ASSERT_FALSE(voteResp["voteID"].empty());
        ASSERT_EQUAL(voteResp["pollID"], pollID);
        ASSERT_EQUAL(voteResp["optionID"], optionID);
        ASSERT_EQUAL(voteResp["userID"], voterID);
        ASSERT_FALSE(voteResp["createdAt"].empty());
    }

    void testSubmitVoteWrongOption() {
        BedrockTester tester = TestHelpers::createTester();
        const string pollID = TestHelpers::createPollID(tester);
        const string voterID = TestHelpers::createUserID(tester, "vote", "Vote", "User");

        SData resp = TestHelpers::submitVote(tester, pollID, "99999", voterID);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testSubmitVoteInvalidPoll() {
        BedrockTester tester = TestHelpers::createTester();
        const string voterID = TestHelpers::createUserID(tester, "vote", "Vote", "User");

        SData resp = TestHelpers::submitVote(tester, "99999", "1", voterID);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "404"));
    }

    void testSubmitVoteInvalidOptionID() {
        BedrockTester tester = TestHelpers::createTester();
        const string pollID = TestHelpers::createPollID(tester);
        const string voterID = TestHelpers::createUserID(tester, "vote", "Vote", "User");

        SData resp = TestHelpers::submitVote(tester, pollID, "0", voterID);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testSubmitVoteInvalidUserID() {
        BedrockTester tester = TestHelpers::createTester();
        const string pollID = TestHelpers::createPollID(tester);
        const STable firstOption = TestHelpers::firstOptionForPoll(tester, pollID);

        SData resp = TestHelpers::submitVote(tester, pollID, firstOption.at("optionID"), "99999");

        ASSERT_TRUE(SStartsWith(resp.methodLine, "404"));
    }

    void testSubmitVoteInvalidUserIDFormat() {
        BedrockTester tester = TestHelpers::createTester();
        const string pollID = TestHelpers::createPollID(tester);
        const STable firstOption = TestHelpers::firstOptionForPoll(tester, pollID);

        SData resp = TestHelpers::submitVote(tester, pollID, firstOption.at("optionID"), "abc");

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testSubmitVoteMissingPollID() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("SubmitVote");
        req["optionID"] = "1";
        req["userID"] = "1";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testSubmitVoteMissingOptionID() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("SubmitVote");
        req["pollID"] = "1";
        req["userID"] = "1";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testSubmitVoteMissingUserID() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("SubmitVote");
        req["pollID"] = "1";
        req["optionID"] = "1";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testSubmitVoteDuplicateUserOnPoll() {
        BedrockTester tester = TestHelpers::createTester();
        const string pollID = TestHelpers::createPollID(tester);
        const string voterID = TestHelpers::createUserID(tester, "vote", "Vote", "User");
        const STable firstOption = TestHelpers::firstOptionForPoll(tester, pollID);
        const string optionID = firstOption.at("optionID");

        SData firstResp = TestHelpers::submitVote(tester, pollID, optionID, voterID);
        ASSERT_TRUE(SStartsWith(firstResp.methodLine, "200 OK"));

        SData secondResp = TestHelpers::submitVote(tester, pollID, optionID, voterID);
        ASSERT_TRUE(SStartsWith(secondResp.methodLine, "409"));
        ASSERT_EQUAL(secondResp["errorCode"], "SUBMIT_VOTE_DUPLICATE_USER");
        ASSERT_EQUAL(SParseJSONObject(secondResp.content).at("errorCode"), "SUBMIT_VOTE_DUPLICATE_USER");
    }

    void testGetPollWithVoteCounts() {
        BedrockTester tester = TestHelpers::createTester();
        const string pollID = TestHelpers::createPollID(tester);

        const STable firstOption = TestHelpers::firstOptionForPoll(tester, pollID);
        const string optionID = firstOption.at("optionID");

        const string voter1 = TestHelpers::createUserID(tester, "vote", "Vote", "One");
        const string voter2 = TestHelpers::createUserID(tester, "vote", "Vote", "Two");
        const string voter3 = TestHelpers::createUserID(tester, "vote", "Vote", "Three");

        for (const string& voterID : {voter1, voter2, voter3}) {
            SData voteResp = TestHelpers::submitVote(tester, pollID, optionID, voterID);
            ASSERT_TRUE(SStartsWith(voteResp.methodLine, "200 OK"));
        }

        SData checkResp = getPoll(tester, pollID);

        ASSERT_TRUE(SStartsWith(checkResp.methodLine, "200 OK"));
        ASSERT_EQUAL(checkResp["totalVotes"], "3");

        list<string> updatedOptions = SParseJSONArray(checkResp["options"]);
        STable updatedFirst = SParseJSONObject(updatedOptions.front());
        ASSERT_EQUAL(updatedFirst["votes"], "3");
    }

    void testEditPollQuestion() {
        BedrockTester tester = TestHelpers::createTester();
        const string createdBy = TestHelpers::createUserID(tester, "polls");
        const string pollID = TestHelpers::createPollID(tester, createdBy);

        SData req("EditPoll");
        req["pollID"] = pollID;
        req["question"] = "Updated question?";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "200 OK"));
        ASSERT_EQUAL(resp["result"], "updated");
        ASSERT_EQUAL(resp["createdBy"], createdBy);

        SData getResp = getPoll(tester, pollID);
        ASSERT_EQUAL(getResp["question"], "Updated question?");
        ASSERT_EQUAL(getResp["createdBy"], createdBy);
    }

    void testEditPollOptions() {
        BedrockTester tester = TestHelpers::createTester();
        const string pollID = TestHelpers::createPollID(tester);
        const string voterID = TestHelpers::createUserID(tester, "vote", "Vote", "User");
        const STable firstOption = TestHelpers::firstOptionForPoll(tester, pollID);

        SData voteResp = TestHelpers::submitVote(tester, pollID, firstOption.at("optionID"), voterID);
        ASSERT_TRUE(SStartsWith(voteResp.methodLine, "200 OK"));

        SData editReq("EditPoll");
        editReq["pollID"] = pollID;
        editReq["options"] = "[\"New A\",\"New B\"]";
        SData editResp = TestHelpers::executeSingle(tester, editReq);

        ASSERT_TRUE(SStartsWith(editResp.methodLine, "200 OK"));
        ASSERT_EQUAL(editResp["optionCount"], "2");

        SData checkResp = getPoll(tester, pollID);
        ASSERT_EQUAL(checkResp["optionCount"], "2");
        ASSERT_EQUAL(checkResp["totalVotes"], "0");
    }

    void testEditPollQuestionAndOptions() {
        BedrockTester tester = TestHelpers::createTester();
        const string pollID = TestHelpers::createPollID(tester);

        SData editReq("EditPoll");
        editReq["pollID"] = pollID;
        editReq["question"] = "Combined update?";
        editReq["options"] = "[\"One\",\"Two\"]";
        SData editResp = TestHelpers::executeSingle(tester, editReq);

        ASSERT_TRUE(SStartsWith(editResp.methodLine, "200 OK"));
        ASSERT_EQUAL(editResp["result"], "updated");
        ASSERT_EQUAL(editResp["optionCount"], "2");

        SData getResp = getPoll(tester, pollID);
        ASSERT_EQUAL(getResp["question"], "Combined update?");
        ASSERT_EQUAL(getResp["optionCount"], "2");
    }

    void testEditPollInvalidID() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("EditPoll");
        req["pollID"] = "0";
        req["question"] = "Doesn't matter";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testEditPollInvalidOptionsJSON() {
        BedrockTester tester = TestHelpers::createTester();
        const string pollID = TestHelpers::createPollID(tester);

        SData req("EditPoll");
        req["pollID"] = pollID;
        req["options"] = "{\"bad\":true}";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testEditPollTooFewOptions() {
        BedrockTester tester = TestHelpers::createTester();
        const string pollID = TestHelpers::createPollID(tester);

        SData req("EditPoll");
        req["pollID"] = pollID;
        req["options"] = "[\"Only one\"]";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testEditPollDuplicateOptions() {
        BedrockTester tester = TestHelpers::createTester();
        const string pollID = TestHelpers::createPollID(tester);

        SData req("EditPoll");
        req["pollID"] = pollID;
        req["options"] = "[\"A\",\"B\",\"A\"]";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testEditPollNoFields() {
        BedrockTester tester = TestHelpers::createTester();
        const string pollID = TestHelpers::createPollID(tester);

        SData req("EditPoll");
        req["pollID"] = pollID;
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testEditPollNotFound() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("EditPoll");
        req["pollID"] = "99999";
        req["question"] = "Doesn't matter";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "404"));
    }

    void testDeletePollSuccess() {
        BedrockTester tester = TestHelpers::createTester();
        const string pollID = TestHelpers::createPollID(tester);

        SData req("DeletePoll");
        req["pollID"] = pollID;
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "200 OK"));
        ASSERT_EQUAL(resp["result"], "deleted");

        SData getResp = getPoll(tester, pollID);
        ASSERT_TRUE(SStartsWith(getResp.methodLine, "404"));
    }

    void testDeletePollNotFound() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("DeletePoll");
        req["pollID"] = "99999";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "404"));
    }

    void testDeletePollInvalidID() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("DeletePoll");
        req["pollID"] = "0";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }
};
