#pragma once

#include "../../../TestHelpers.h"

#include <iterator>
#include <map>

struct PollFeaturesTest : tpunit::TestFixture {
    PollFeaturesTest()
        : tpunit::TestFixture(
            "PollFeaturesTests",
            TEST(PollFeaturesTest::testChoicePollOptionLimitsAreEnforced),
            TEST(PollFeaturesTest::testRankedChoiceRejectsDuplicateOptionIDs),
            TEST(PollFeaturesTest::testRankedChoiceRejectsOptionOutsidePoll),
            TEST(PollFeaturesTest::testRankedChoiceGetUsesFirstChoiceCountsOnly),
            TEST(PollFeaturesTest::testParticipationForRankedNonAnonymousPoll),
            TEST(PollFeaturesTest::testParticipationForAnonymousFreeTextPoll),
            TEST(PollFeaturesTest::testManualCloseCreatesSingleSummaryAndListIncludesLink),
            TEST(PollFeaturesTest::testExpiryCloseCreatesSingleSummary)
        ) { }

    static string buildOptionsJSON(size_t count) {
        list<string> labels;
        for (size_t i = 1; i <= count; ++i) {
            labels.emplace_back("Option " + SToStr(i));
        }
        return SComposeJSONArray(labels);
    }

    static string createPollInChat(BedrockTester& tester,
                                   const string& chatID,
                                   const string& creatorUserID,
                                   const string& type,
                                   const string& options = "[\"A\",\"B\",\"C\"]",
                                   bool isAnonymous = false,
                                   const string& expiresAt = "") {
        SData req("CreatePoll");
        req["chatID"] = chatID;
        req["creatorUserID"] = creatorUserID;
        req["question"] = "Feature test poll?";
        req["type"] = type;
        req["isAnonymous"] = isAnonymous ? "true" : "false";
        if (type != "free_text") {
            req["options"] = options;
        }
        if (!expiresAt.empty()) {
            req["expiresAt"] = expiresAt;
        }

        SData resp = TestHelpers::executeSingle(tester, req);
        if (!SStartsWith(resp.methodLine, "200 OK")) {
            STHROW("PollFeaturesTest createPollInChat failed: " + resp.methodLine);
        }
        return resp["pollID"];
    }

    static list<string> optionIDsForPoll(BedrockTester& tester,
                                         const string& pollID,
                                         const string& requesterUserID) {
        const SData poll = TestHelpers::getPoll(tester, pollID, requesterUserID);
        list<string> optionRows = SParseJSONArray(poll["options"]);
        list<string> optionIDs;
        for (const string& row : optionRows) {
            optionIDs.emplace_back(SParseJSONObject(row).at("optionID"));
        }
        return optionIDs;
    }

    void testChoicePollOptionLimitsAreEnforced() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "limit-owner", "Limit", "Owner");
        const string chatID = TestHelpers::createChatID(tester, creatorID, "Limits");

        SData tooFewReq("CreatePoll");
        tooFewReq["chatID"] = chatID;
        tooFewReq["creatorUserID"] = creatorID;
        tooFewReq["question"] = "Too few";
        tooFewReq["type"] = "single_choice";
        tooFewReq["options"] = "[\"Only\"]";
        const SData tooFewResp = TestHelpers::executeSingle(tester, tooFewReq);
        ASSERT_TRUE(SStartsWith(tooFewResp.methodLine, "400"));

        SData tooManyReq("CreatePoll");
        tooManyReq["chatID"] = chatID;
        tooManyReq["creatorUserID"] = creatorID;
        tooManyReq["question"] = "Too many";
        tooManyReq["type"] = "single_choice";
        tooManyReq["options"] = buildOptionsJSON(21);
        const SData tooManyResp = TestHelpers::executeSingle(tester, tooManyReq);
        ASSERT_TRUE(SStartsWith(tooManyResp.methodLine, "400"));

        SData maxAllowedReq("CreatePoll");
        maxAllowedReq["chatID"] = chatID;
        maxAllowedReq["creatorUserID"] = creatorID;
        maxAllowedReq["question"] = "Max allowed";
        maxAllowedReq["type"] = "single_choice";
        maxAllowedReq["options"] = buildOptionsJSON(20);
        const SData maxAllowedResp = TestHelpers::executeSingle(tester, maxAllowedReq);
        ASSERT_TRUE(SStartsWith(maxAllowedResp.methodLine, "200 OK"));
        ASSERT_EQUAL(maxAllowedResp["optionCount"], "20");
    }

    void testRankedChoiceRejectsDuplicateOptionIDs() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "ranked-dup-owner", "Ranked", "Owner");
        const string voterID = TestHelpers::createUserID(tester, "ranked-dup-voter", "Ranked", "Voter");
        const string chatID = TestHelpers::createChatID(tester, creatorID, "Ranked dup");
        const string pollID = createPollInChat(tester, chatID, creatorID, "ranked_choice", "[\"A\",\"B\",\"C\"]");
        const list<string> optionIDs = optionIDsForPoll(tester, pollID, creatorID);
        ASSERT_EQUAL(optionIDs.size(), static_cast<size_t>(3));

        SData req("SubmitPollVotes");
        req["pollID"] = pollID;
        req["userID"] = voterID;
        req["optionIDs"] = SComposeJSONArray(
            list<string> {optionIDs.front(), optionIDs.front(), *next(optionIDs.begin())}
        );
        const SData resp = TestHelpers::executeSingle(tester, req);
        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testRankedChoiceRejectsOptionOutsidePoll() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "ranked-outside-owner", "Ranked", "Owner");
        const string voterID = TestHelpers::createUserID(tester, "ranked-outside-voter", "Ranked", "Voter");
        const string chatID = TestHelpers::createChatID(tester, creatorID, "Ranked outside");
        const string pollID = createPollInChat(tester, chatID, creatorID, "ranked_choice", "[\"A\",\"B\",\"C\"]");
        const list<string> optionIDs = optionIDsForPoll(tester, pollID, creatorID);
        ASSERT_EQUAL(optionIDs.size(), static_cast<size_t>(3));

        const SData resp = TestHelpers::submitVotes(
            tester,
            pollID,
            {optionIDs.front(), *next(optionIDs.begin()), "999999"},
            voterID
        );
        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testRankedChoiceGetUsesFirstChoiceCountsOnly() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "ranked-count-owner", "Ranked", "Owner");
        const string voterOneID = TestHelpers::createUserID(tester, "ranked-count-v1", "Ranked", "V1");
        const string voterTwoID = TestHelpers::createUserID(tester, "ranked-count-v2", "Ranked", "V2");
        const string chatID = TestHelpers::createChatID(tester, creatorID, "Ranked count");
        const string pollID = createPollInChat(
            tester,
            chatID,
            creatorID,
            "ranked_choice",
            "[\"Alpha\",\"Beta\",\"Gamma\"]"
        );
        const list<string> optionIDs = optionIDsForPoll(tester, pollID, creatorID);
        ASSERT_EQUAL(optionIDs.size(), static_cast<size_t>(3));

        const string alphaID = optionIDs.front();
        const string betaID = *next(optionIDs.begin());
        const string gammaID = *next(optionIDs.begin(), 2);

        const SData voteOneResp = TestHelpers::submitVotes(tester, pollID, {alphaID, betaID, gammaID}, voterOneID);
        ASSERT_TRUE(SStartsWith(voteOneResp.methodLine, "200 OK"));
        const SData voteTwoResp = TestHelpers::submitVotes(tester, pollID, {betaID, alphaID, gammaID}, voterTwoID);
        ASSERT_TRUE(SStartsWith(voteTwoResp.methodLine, "200 OK"));

        const SData pollResp = TestHelpers::getPoll(tester, pollID, creatorID);
        ASSERT_TRUE(SStartsWith(pollResp.methodLine, "200 OK"));
        ASSERT_EQUAL(pollResp["totalVotes"], "2");
        ASSERT_EQUAL(pollResp["totalVoters"], "2");

        list<string> optionRows = SParseJSONArray(pollResp["options"]);
        map<string, string> voteCountsByLabel;
        for (const string& optionRow : optionRows) {
            const STable option = SParseJSONObject(optionRow);
            voteCountsByLabel[option.at("label")] = option.at("voteCount");
            ASSERT_TRUE(!option.contains("votePct"));
        }

        ASSERT_EQUAL(voteCountsByLabel["Alpha"], "1");
        ASSERT_EQUAL(voteCountsByLabel["Beta"], "1");
        ASSERT_EQUAL(voteCountsByLabel["Gamma"], "0");
    }

    void testParticipationForRankedNonAnonymousPoll() {
        BedrockTester tester = TestHelpers::createTester();
        const string ownerID = TestHelpers::createUserID(tester, "part-ranked-owner", "Part", "Owner");
        const string voterID = TestHelpers::createUserID(tester, "part-ranked-voter", "Part", "Voter");
        const string nonVoterID = TestHelpers::createUserID(tester, "part-ranked-nonvoter", "Part", "NonVoter");
        const string chatID = TestHelpers::createChatID(tester, ownerID, "Part ranked");
        TestHelpers::addChatMember(tester, chatID, ownerID, voterID, "member");
        TestHelpers::addChatMember(tester, chatID, ownerID, nonVoterID, "member");
        const string pollID = createPollInChat(tester, chatID, ownerID, "ranked_choice", "[\"A\",\"B\",\"C\"]");
        const list<string> optionIDs = optionIDsForPoll(tester, pollID, ownerID);
        (void)TestHelpers::submitVotes(tester, pollID, optionIDs, voterID);

        SData req("GetPollParticipation");
        req["pollID"] = pollID;
        req["requesterUserID"] = ownerID;
        const SData resp = TestHelpers::executeSingle(tester, req);
        ASSERT_TRUE(SStartsWith(resp.methodLine, "200 OK"));
        ASSERT_EQUAL(resp["eligibleCount"], "3");
        ASSERT_EQUAL(resp["votedCount"], "1");
        ASSERT_EQUAL(resp["notVotedCount"], "2");

        const list<string> votedUserIDs = SParseJSONArray(resp["votedUserIDs"]);
        ASSERT_EQUAL(votedUserIDs.size(), static_cast<size_t>(1));
        ASSERT_EQUAL(votedUserIDs.front(), voterID);
    }

    void testParticipationForAnonymousFreeTextPoll() {
        BedrockTester tester = TestHelpers::createTester();
        const string ownerID = TestHelpers::createUserID(tester, "part-anon-owner", "Part", "Owner");
        const string responderID = TestHelpers::createUserID(tester, "part-anon-responder", "Part", "Responder");
        const string chatID = TestHelpers::createChatID(tester, ownerID, "Part anon");
        TestHelpers::addChatMember(tester, chatID, ownerID, responderID, "member");
        const string pollID = createPollInChat(tester, chatID, ownerID, "free_text", "", true);
        (void)TestHelpers::submitTextResponse(tester, pollID, responderID, "Anonymous text response");

        SData req("GetPollParticipation");
        req["pollID"] = pollID;
        req["requesterUserID"] = ownerID;
        const SData resp = TestHelpers::executeSingle(tester, req);
        ASSERT_TRUE(SStartsWith(resp.methodLine, "200 OK"));
        ASSERT_EQUAL(resp["isAnonymous"], "true");
        ASSERT_EQUAL(resp["eligibleCount"], "2");
        ASSERT_EQUAL(resp["votedCount"], "1");
        ASSERT_EQUAL(resp["notVotedCount"], "1");
        ASSERT_TRUE(resp["eligibleUserIDs"].empty());
        ASSERT_TRUE(resp["votedUserIDs"].empty());
        ASSERT_TRUE(resp["notVotedUserIDs"].empty());
    }

    void testManualCloseCreatesSingleSummaryAndListIncludesLink() {
        BedrockTester tester = TestHelpers::createTester();
        const string ownerID = TestHelpers::createUserID(tester, "summary-manual-owner-2", "Summary", "Owner");
        const string chatID = TestHelpers::createChatID(tester, ownerID, "Summary manual");
        const string pollID = createPollInChat(tester, chatID, ownerID, "single_choice", "[\"A\",\"B\"]");

        SData closeReq("EditPoll");
        closeReq["pollID"] = pollID;
        closeReq["actorUserID"] = ownerID;
        closeReq["status"] = "closed";
        const SData closeResp = TestHelpers::executeSingle(tester, closeReq);
        ASSERT_TRUE(SStartsWith(closeResp.methodLine, "200 OK"));

        const SData firstGetResp = TestHelpers::getPoll(tester, pollID, ownerID);
        ASSERT_TRUE(SStartsWith(firstGetResp.methodLine, "200 OK"));
        ASSERT_FALSE(firstGetResp["summaryMessageID"].empty());
        const string summaryMessageID = firstGetResp["summaryMessageID"];

        const SData listResp = TestHelpers::listPolls(tester, chatID, ownerID, true);
        ASSERT_TRUE(SStartsWith(listResp.methodLine, "200 OK"));
        const list<string> rows = SParseJSONArray(listResp["polls"]);
        ASSERT_FALSE(rows.empty());
        const STable pollRow = SParseJSONObject(rows.front());
        ASSERT_EQUAL(pollRow.at("pollID"), pollID);
        ASSERT_EQUAL(pollRow.at("summaryMessageID"), summaryMessageID);

        const string firstCount = tester.readDB(
            "SELECT COUNT(*) FROM poll_summary_messages WHERE pollID = " + pollID + ";"
        );
        ASSERT_EQUAL(firstCount, "1");

        const SData closeAgainResp = TestHelpers::executeSingle(tester, closeReq);
        ASSERT_TRUE(SStartsWith(closeAgainResp.methodLine, "200 OK"));
        const string secondCount = tester.readDB(
            "SELECT COUNT(*) FROM poll_summary_messages WHERE pollID = " + pollID + ";"
        );
        ASSERT_EQUAL(secondCount, "1");
    }

    void testExpiryCloseCreatesSingleSummary() {
        BedrockTester tester = TestHelpers::createTester();
        const string ownerID = TestHelpers::createUserID(tester, "summary-expiry-owner-2", "Summary", "Owner");
        const string chatID = TestHelpers::createChatID(tester, ownerID, "Summary expiry");
        const string expiredAt = SToStr(STimeNow() - 1);
        const string pollID = createPollInChat(
            tester,
            chatID,
            ownerID,
            "single_choice",
            "[\"Now\",\"Later\"]",
            false,
            expiredAt
        );

        const SData firstGetResp = TestHelpers::getPoll(tester, pollID, ownerID);
        ASSERT_TRUE(SStartsWith(firstGetResp.methodLine, "200 OK"));
        ASSERT_EQUAL(firstGetResp["status"], "closed");
        ASSERT_FALSE(firstGetResp["summaryMessageID"].empty());

        const string firstCount = tester.readDB(
            "SELECT COUNT(*) FROM poll_summary_messages WHERE pollID = " + pollID + ";"
        );
        ASSERT_EQUAL(firstCount, "1");

        const SData secondGetResp = TestHelpers::getPoll(tester, pollID, ownerID);
        ASSERT_TRUE(SStartsWith(secondGetResp.methodLine, "200 OK"));
        ASSERT_EQUAL(secondGetResp["summaryMessageID"], firstGetResp["summaryMessageID"]);

        const string secondCount = tester.readDB(
            "SELECT COUNT(*) FROM poll_summary_messages WHERE pollID = " + pollID + ";"
        );
        ASSERT_EQUAL(secondCount, "1");
    }
};
