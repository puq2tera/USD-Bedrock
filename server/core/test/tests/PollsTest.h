#pragma once

#include "../TestHelpers.h"
#include <libstuff/SData.h>

struct PollsTest : tpunit::TestFixture {
    PollsTest()
        : tpunit::TestFixture(
            "PollsTests",

            // CreatePoll
            TEST(PollsTest::testCreatePollSuccess),
            TEST(PollsTest::testCreatePollMissingQuestion),
            TEST(PollsTest::testCreatePollTooFewOptions),
            TEST(PollsTest::testCreatePollTooManyOptions),
            TEST(PollsTest::testCreatePollEmptyOptionText),
            TEST(PollsTest::testCreatePollDuplicateOptions),

            // GetPoll
            TEST(PollsTest::testGetPollSuccess),
            TEST(PollsTest::testGetPollNotFound),
            TEST(PollsTest::testGetPollInvalidID),

            // SubmitVote
            TEST(PollsTest::testSubmitVoteSuccess),
            TEST(PollsTest::testSubmitVoteWrongOption),
            TEST(PollsTest::testSubmitVoteInvalidPoll),

            // GetPoll with vote counts
            TEST(PollsTest::testGetPollWithVoteCounts),

            // EditPoll
            TEST(PollsTest::testEditPollQuestion),
            TEST(PollsTest::testEditPollOptions),
            TEST(PollsTest::testEditPollNoFields),
            TEST(PollsTest::testEditPollNotFound),

            // DeletePoll
            TEST(PollsTest::testDeletePollSuccess),
            TEST(PollsTest::testDeletePollNotFound)
        ) { }

    // ==================== Helper ====================

    // Creates a poll and returns the pollID
    string createTestPoll(BedrockTester& tester, const string& question = "Test question?",
                          const string& options = "[\"Option A\",\"Option B\",\"Option C\"]") {
        SData req("CreatePoll");
        req["question"] = question;
        req["options"] = options;
        SData resp = tester.executeWaitMultipleData({req}, 1).front();
        if (!SStartsWith(resp.methodLine, "200 OK")) {
            STHROW("Test helper createTestPoll failed: " + resp.methodLine);
        }
        return resp["pollID"];
    }

    // ==================== CreatePoll ====================

    void testCreatePollSuccess() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("CreatePoll");
        req["question"] = "What is your favorite color?";
        req["options"] = "[\"Red\",\"Blue\",\"Green\"]";
        SData resp = tester.executeWaitMultipleData({req}, 1).front();

        ASSERT_TRUE(SStartsWith(resp.methodLine, "200 OK"));
        ASSERT_FALSE(resp["pollID"].empty());
        ASSERT_EQUAL(resp["question"], "What is your favorite color?");
        ASSERT_EQUAL(resp["optionCount"], "3");
        ASSERT_FALSE(resp["createdAt"].empty());
    }

    void testCreatePollMissingQuestion() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("CreatePoll");
        req["options"] = "[\"A\",\"B\"]";
        SData resp = tester.executeWaitMultipleData({req}, 1).front();

        ASSERT_TRUE(SStartsWith(resp.methodLine, "402"));
    }

    void testCreatePollTooFewOptions() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("CreatePoll");
        req["question"] = "Only one option?";
        req["options"] = "[\"Lonely\"]";
        SData resp = tester.executeWaitMultipleData({req}, 1).front();

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testCreatePollTooManyOptions() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("CreatePoll");
        req["question"] = "Too many?";
        req["options"] = "[\"1\",\"2\",\"3\",\"4\",\"5\",\"6\",\"7\",\"8\",\"9\",\"10\","
                         "\"11\",\"12\",\"13\",\"14\",\"15\",\"16\",\"17\",\"18\",\"19\",\"20\",\"21\"]";
        SData resp = tester.executeWaitMultipleData({req}, 1).front();

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testCreatePollEmptyOptionText() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("CreatePoll");
        req["question"] = "Empty option?";
        req["options"] = "[\"Good\",\"\",\"Bad\"]";
        SData resp = tester.executeWaitMultipleData({req}, 1).front();

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testCreatePollDuplicateOptions() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("CreatePoll");
        req["question"] = "Duplicates?";
        req["options"] = "[\"Apple\",\"Banana\",\"Apple\"]";
        SData resp = tester.executeWaitMultipleData({req}, 1).front();

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    // ==================== GetPoll ====================

    void testGetPollSuccess() {
        BedrockTester tester = TestHelpers::createTester();
        string pollID = createTestPoll(tester);

        SData req("GetPoll");
        req["pollID"] = pollID;
        SData resp = tester.executeWaitMultipleData({req}, 1).front();

        ASSERT_TRUE(SStartsWith(resp.methodLine, "200 OK"));
        ASSERT_EQUAL(resp["pollID"], pollID);
        ASSERT_EQUAL(resp["question"], "Test question?");
        ASSERT_EQUAL(resp["optionCount"], "3");
        ASSERT_EQUAL(resp["totalVotes"], "0");
        ASSERT_FALSE(resp["options"].empty());
    }

    void testGetPollNotFound() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("GetPoll");
        req["pollID"] = "99999";
        SData resp = tester.executeWaitMultipleData({req}, 1).front();

        ASSERT_TRUE(SStartsWith(resp.methodLine, "404"));
    }

    void testGetPollInvalidID() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("GetPoll");
        req["pollID"] = "0";
        SData resp = tester.executeWaitMultipleData({req}, 1).front();

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    // ==================== SubmitVote ====================

    void testSubmitVoteSuccess() {
        BedrockTester tester = TestHelpers::createTester();
        string pollID = createTestPoll(tester);

        // Get the first optionID
        SData getReq("GetPoll");
        getReq["pollID"] = pollID;
        SData getResp = tester.executeWaitMultipleData({getReq}, 1).front();
        list<string> optionsList = SParseJSONArray(getResp["options"]);
        STable firstOption = SParseJSONObject(optionsList.front());
        string optionID = firstOption["optionID"];

        SData voteReq("SubmitVote");
        voteReq["pollID"] = pollID;
        voteReq["optionID"] = optionID;
        SData voteResp = tester.executeWaitMultipleData({voteReq}, 1).front();

        ASSERT_TRUE(SStartsWith(voteResp.methodLine, "200 OK"));
        ASSERT_FALSE(voteResp["voteID"].empty());
        ASSERT_EQUAL(voteResp["pollID"], pollID);
        ASSERT_EQUAL(voteResp["optionID"], optionID);
    }

    void testSubmitVoteWrongOption() {
        BedrockTester tester = TestHelpers::createTester();
        string pollID = createTestPoll(tester);

        SData req("SubmitVote");
        req["pollID"] = pollID;
        req["optionID"] = "99999";
        SData resp = tester.executeWaitMultipleData({req}, 1).front();

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testSubmitVoteInvalidPoll() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("SubmitVote");
        req["pollID"] = "99999";
        req["optionID"] = "1";
        SData resp = tester.executeWaitMultipleData({req}, 1).front();

        ASSERT_TRUE(SStartsWith(resp.methodLine, "404"));
    }

    // ==================== GetPoll with vote counts ====================

    void testGetPollWithVoteCounts() {
        BedrockTester tester = TestHelpers::createTester();
        string pollID = createTestPoll(tester);

        // Get option IDs
        SData getReq("GetPoll");
        getReq["pollID"] = pollID;
        SData getResp = tester.executeWaitMultipleData({getReq}, 1).front();
        list<string> optionsList = SParseJSONArray(getResp["options"]);
        STable firstOption = SParseJSONObject(optionsList.front());
        string optionID = firstOption["optionID"];

        // Cast 3 votes on the first option
        for (int i = 0; i < 3; i++) {
            SData voteReq("SubmitVote");
            voteReq["pollID"] = pollID;
            voteReq["optionID"] = optionID;
            tester.executeWaitMultipleData({voteReq}, 1);
        }

        // Fetch and verify counts
        SData checkReq("GetPoll");
        checkReq["pollID"] = pollID;
        SData checkResp = tester.executeWaitMultipleData({checkReq}, 1).front();

        ASSERT_TRUE(SStartsWith(checkResp.methodLine, "200 OK"));
        ASSERT_EQUAL(checkResp["totalVotes"], "3");

        // Verify the first option has 3 votes
        list<string> updatedOptions = SParseJSONArray(checkResp["options"]);
        STable updatedFirst = SParseJSONObject(updatedOptions.front());
        ASSERT_EQUAL(updatedFirst["votes"], "3");
    }

    // ==================== EditPoll ====================

    void testEditPollQuestion() {
        BedrockTester tester = TestHelpers::createTester();
        string pollID = createTestPoll(tester);

        SData req("EditPoll");
        req["pollID"] = pollID;
        req["question"] = "Updated question?";
        SData resp = tester.executeWaitMultipleData({req}, 1).front();

        ASSERT_TRUE(SStartsWith(resp.methodLine, "200 OK"));
        ASSERT_EQUAL(resp["result"], "updated");

        // Verify the question changed
        SData getReq("GetPoll");
        getReq["pollID"] = pollID;
        SData getResp = tester.executeWaitMultipleData({getReq}, 1).front();
        ASSERT_EQUAL(getResp["question"], "Updated question?");
    }

    void testEditPollOptions() {
        BedrockTester tester = TestHelpers::createTester();
        string pollID = createTestPoll(tester);

        // Vote first, so we can verify votes get cleared
        SData getReq("GetPoll");
        getReq["pollID"] = pollID;
        SData getResp = tester.executeWaitMultipleData({getReq}, 1).front();
        list<string> optionsList = SParseJSONArray(getResp["options"]);
        STable firstOption = SParseJSONObject(optionsList.front());

        SData voteReq("SubmitVote");
        voteReq["pollID"] = pollID;
        voteReq["optionID"] = firstOption["optionID"];
        tester.executeWaitMultipleData({voteReq}, 1);

        // Now edit options
        SData editReq("EditPoll");
        editReq["pollID"] = pollID;
        editReq["options"] = "[\"New A\",\"New B\"]";
        SData editResp = tester.executeWaitMultipleData({editReq}, 1).front();

        ASSERT_TRUE(SStartsWith(editResp.methodLine, "200 OK"));
        ASSERT_EQUAL(editResp["optionCount"], "2");

        // Verify options changed and votes were cleared
        SData checkReq("GetPoll");
        checkReq["pollID"] = pollID;
        SData checkResp = tester.executeWaitMultipleData({checkReq}, 1).front();
        ASSERT_EQUAL(checkResp["optionCount"], "2");
        ASSERT_EQUAL(checkResp["totalVotes"], "0");
    }

    void testEditPollNoFields() {
        BedrockTester tester = TestHelpers::createTester();
        string pollID = createTestPoll(tester);

        SData req("EditPoll");
        req["pollID"] = pollID;
        SData resp = tester.executeWaitMultipleData({req}, 1).front();

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testEditPollNotFound() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("EditPoll");
        req["pollID"] = "99999";
        req["question"] = "Doesn't matter";
        SData resp = tester.executeWaitMultipleData({req}, 1).front();

        ASSERT_TRUE(SStartsWith(resp.methodLine, "404"));
    }

    // ==================== DeletePoll ====================

    void testDeletePollSuccess() {
        BedrockTester tester = TestHelpers::createTester();
        string pollID = createTestPoll(tester);

        SData req("DeletePoll");
        req["pollID"] = pollID;
        SData resp = tester.executeWaitMultipleData({req}, 1).front();

        ASSERT_TRUE(SStartsWith(resp.methodLine, "200 OK"));
        ASSERT_EQUAL(resp["result"], "deleted");

        // Verify it's actually gone
        SData getReq("GetPoll");
        getReq["pollID"] = pollID;
        SData getResp = tester.executeWaitMultipleData({getReq}, 1).front();
        ASSERT_TRUE(SStartsWith(getResp.methodLine, "404"));
    }

    void testDeletePollNotFound() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("DeletePoll");
        req["pollID"] = "99999";
        SData resp = tester.executeWaitMultipleData({req}, 1).front();

        ASSERT_TRUE(SStartsWith(resp.methodLine, "404"));
    }
};
