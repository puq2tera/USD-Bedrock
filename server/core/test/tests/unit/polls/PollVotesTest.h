#pragma once

#include "../../../TestHelpers.h"

#include <iterator>

struct PollVotesTest : tpunit::TestFixture {
    PollVotesTest()
        : tpunit::TestFixture(
            "PollVotesTests",
            TEST(PollVotesTest::testSubmitVoteSuccess),
            TEST(PollVotesTest::testSubmitVotesRequiresChatMembership),
            TEST(PollVotesTest::testSubmitVotesWrongTypeForFreeText),
            TEST(PollVotesTest::testSubmitVotesSingleChoiceRequiresExactlyOneOption),
            TEST(PollVotesTest::testSubmitVotesRejectsDuplicateOptionIDs),
            TEST(PollVotesTest::testSubmitVotesRejectsOptionOutsidePoll),
            TEST(PollVotesTest::testSubmitVotesRespectsAllowChangeVote),
            TEST(PollVotesTest::testDeletePollVotesSuccessAndNoOp),
            TEST(PollVotesTest::testDeletePollVotesWrongTypeForFreeText),
            TEST(PollVotesTest::testDeletePollVotesRejectsClosedPoll)
        ) { }

    string createChoicePoll(BedrockTester& tester,
                            const string& creatorUserID,
                            const string& type,
                            bool allowChangeVote = false) {
        const string chatID = TestHelpers::createChatID(tester, creatorUserID, "Poll votes chat");

        SData req("CreatePoll");
        req["chatID"] = chatID;
        req["creatorUserID"] = creatorUserID;
        req["question"] = "Votes?";
        req["type"] = type;
        req["options"] = "[\"A\",\"B\",\"C\"]";
        req["allowChangeVote"] = allowChangeVote ? "true" : "false";
        SData resp = TestHelpers::executeSingle(tester, req);

        EXPECT_TRUE(SStartsWith(resp.methodLine, "200 OK"));
        return resp["pollID"];
    }

    list<string> optionIDsForPoll(BedrockTester& tester, const string& pollID, const string& requesterUserID) {
        const SData poll = TestHelpers::getPoll(tester, pollID, requesterUserID);
        list<string> optionRows = SParseJSONArray(poll["options"]);
        list<string> optionIDs;
        for (const string& row : optionRows) {
            optionIDs.emplace_back(SParseJSONObject(row).at("optionID"));
        }
        return optionIDs;
    }

    void testSubmitVoteSuccess() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "vote-owner", "Vote", "Owner");
        const string voterID = TestHelpers::createUserID(tester, "vote-user", "Vote", "User");
        const string pollID = createChoicePoll(tester, creatorID, "single_choice");

        const string optionID = optionIDsForPoll(tester, pollID, creatorID).front();
        SData submitResp = TestHelpers::submitVote(tester, pollID, optionID, voterID);

        ASSERT_TRUE(SStartsWith(submitResp.methodLine, "200 OK"));
        ASSERT_EQUAL(submitResp["submittedCount"], "1");
        ASSERT_EQUAL(submitResp["replaced"], "false");

        const SData pollResp = TestHelpers::getPoll(tester, pollID, creatorID);
        ASSERT_EQUAL(pollResp["totalVotes"], "1");
        ASSERT_EQUAL(pollResp["totalVoters"], "1");
    }

    void testSubmitVotesRequiresChatMembership() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "vote-owner", "Vote", "Owner");
        const string outsiderID = TestHelpers::createUserID(tester, "vote-outsider", "Vote", "Outsider");
        const string pollID = createChoicePoll(tester, creatorID, "single_choice");
        const string optionID = optionIDsForPoll(tester, pollID, creatorID).front();

        SData req("SubmitPollVotes");
        req["pollID"] = pollID;
        req["userID"] = outsiderID;
        req["optionIDs"] = SComposeJSONArray(list<string> {optionID});
        const SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "409"));
    }

    void testSubmitVotesWrongTypeForFreeText() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "vote-free-text", "Vote", "Creator");
        const string pollID = TestHelpers::createPollID(
            tester,
            creatorID,
            "free text",
            "[\"unused\",\"unused2\"]",
            "free_text"
        );
        const string voterID = TestHelpers::createUserID(tester, "vote-free-text-user", "Vote", "User");

        const SData resp = TestHelpers::submitVotes(tester, pollID, {"1"}, voterID);
        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testSubmitVotesSingleChoiceRequiresExactlyOneOption() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "single-owner", "Single", "Owner");
        const string voterID = TestHelpers::createUserID(tester, "single-voter", "Single", "Voter");
        const string pollID = createChoicePoll(tester, creatorID, "single_choice");
        const list<string> optionIDs = optionIDsForPoll(tester, pollID, creatorID);
        ASSERT_TRUE(optionIDs.size() >= 2);

        const string firstOptionID = optionIDs.front();
        const string secondOptionID = *next(optionIDs.begin());
        const SData resp = TestHelpers::submitVotes(tester, pollID, {firstOptionID, secondOptionID}, voterID);
        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testSubmitVotesRejectsDuplicateOptionIDs() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "duplicate-owner", "Duplicate", "Owner");
        const string voterID = TestHelpers::createUserID(tester, "duplicate-voter", "Duplicate", "Voter");
        const string pollID = createChoicePoll(tester, creatorID, "multiple_choice");
        const string optionID = optionIDsForPoll(tester, pollID, creatorID).front();

        SData req("SubmitPollVotes");
        req["pollID"] = pollID;
        req["userID"] = voterID;
        req["optionIDs"] = SComposeJSONArray(list<string> {optionID, optionID});
        const SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testSubmitVotesRejectsOptionOutsidePoll() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "outside-owner", "Outside", "Owner");
        const string voterID = TestHelpers::createUserID(tester, "outside-voter", "Outside", "Voter");
        const string pollID = createChoicePoll(tester, creatorID, "single_choice");

        const SData resp = TestHelpers::submitVotes(tester, pollID, {"999999"}, voterID);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testSubmitVotesRespectsAllowChangeVote() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "allow-owner", "Allow", "Owner");
        const string voterID = TestHelpers::createUserID(tester, "allow-voter", "Allow", "Voter");

        const string noChangePollID = createChoicePoll(tester, creatorID, "multiple_choice", false);
        const list<string> noChangeOptions = optionIDsForPoll(tester, noChangePollID, creatorID);
        const string noChangeFirstOption = noChangeOptions.front();
        const string noChangeSecondOption = *next(noChangeOptions.begin());

        const SData firstNoChange = TestHelpers::submitVote(tester, noChangePollID, noChangeFirstOption, voterID);
        ASSERT_TRUE(SStartsWith(firstNoChange.methodLine, "200 OK"));

        const SData secondNoChange = TestHelpers::submitVote(tester, noChangePollID, noChangeSecondOption, voterID);
        ASSERT_TRUE(SStartsWith(secondNoChange.methodLine, "409"));

        const string allowChangePollID = createChoicePoll(tester, creatorID, "multiple_choice", true);
        const list<string> allowOptions = optionIDsForPoll(tester, allowChangePollID, creatorID);
        const string allowFirstOption = allowOptions.front();
        const string allowSecondOption = *next(allowOptions.begin());

        const SData firstAllow = TestHelpers::submitVote(tester, allowChangePollID, allowFirstOption, voterID);
        ASSERT_TRUE(SStartsWith(firstAllow.methodLine, "200 OK"));

        const SData secondAllow = TestHelpers::submitVote(tester, allowChangePollID, allowSecondOption, voterID);
        ASSERT_TRUE(SStartsWith(secondAllow.methodLine, "200 OK"));
        ASSERT_EQUAL(secondAllow["replaced"], "true");
    }

    void testDeletePollVotesSuccessAndNoOp() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "delete-votes-owner", "Delete", "Owner");
        const string voterID = TestHelpers::createUserID(tester, "delete-votes-user", "Delete", "User");
        const string pollID = createChoicePoll(tester, creatorID, "multiple_choice");
        const string optionID = optionIDsForPoll(tester, pollID, creatorID).front();

        const SData submitResp = TestHelpers::submitVote(tester, pollID, optionID, voterID);
        ASSERT_TRUE(SStartsWith(submitResp.methodLine, "200 OK"));

        SData firstDeleteReq("DeletePollVotes");
        firstDeleteReq["pollID"] = pollID;
        firstDeleteReq["userID"] = voterID;
        const SData firstDeleteResp = TestHelpers::executeSingle(tester, firstDeleteReq);
        ASSERT_TRUE(SStartsWith(firstDeleteResp.methodLine, "200 OK"));
        ASSERT_EQUAL(firstDeleteResp["removedCount"], "1");

        const SData secondDeleteResp = TestHelpers::executeSingle(tester, firstDeleteReq);
        ASSERT_TRUE(SStartsWith(secondDeleteResp.methodLine, "200 OK"));
        ASSERT_EQUAL(secondDeleteResp["removedCount"], "0");

        const SData pollResp = TestHelpers::getPoll(tester, pollID, creatorID);
        ASSERT_EQUAL(pollResp["totalVotes"], "0");
    }

    void testDeletePollVotesWrongTypeForFreeText() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "delete-free-text", "Delete", "FreeText");
        const string pollID = TestHelpers::createPollID(
            tester,
            creatorID,
            "free text",
            "[\"unused\",\"unused2\"]",
            "free_text"
        );

        SData req("DeletePollVotes");
        req["pollID"] = pollID;
        req["userID"] = creatorID;
        const SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testDeletePollVotesRejectsClosedPoll() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "delete-closed-owner", "Delete", "ClosedOwner");
        const string voterID = TestHelpers::createUserID(tester, "delete-closed-voter", "Delete", "ClosedVoter");
        const string pollID = createChoicePoll(tester, creatorID, "single_choice");
        const string optionID = optionIDsForPoll(tester, pollID, creatorID).front();

        const SData submitResp = TestHelpers::submitVote(tester, pollID, optionID, voterID);
        ASSERT_TRUE(SStartsWith(submitResp.methodLine, "200 OK"));

        SData closeReq("EditPoll");
        closeReq["pollID"] = pollID;
        closeReq["actorUserID"] = creatorID;
        closeReq["status"] = "closed";
        const SData closeResp = TestHelpers::executeSingle(tester, closeReq);
        ASSERT_TRUE(SStartsWith(closeResp.methodLine, "200 OK"));

        SData deleteReq("DeletePollVotes");
        deleteReq["pollID"] = pollID;
        deleteReq["userID"] = voterID;
        const SData deleteResp = TestHelpers::executeSingle(tester, deleteReq);

        ASSERT_TRUE(SStartsWith(deleteResp.methodLine, "409"));
    }
};
