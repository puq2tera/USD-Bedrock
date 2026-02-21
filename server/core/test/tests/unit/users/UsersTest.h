#pragma once

#include "../../../TestHelpers.h"

struct UsersTest : tpunit::TestFixture {
    UsersTest()
        : tpunit::TestFixture(
            "UsersTests",
            TEST(UsersTest::testCreateUserSuccess),
            TEST(UsersTest::testCreateUserMissingEmail),
            TEST(UsersTest::testCreateUserInvalidEmail),
            TEST(UsersTest::testCreateUserMissingFirstName),
            TEST(UsersTest::testCreateUserMissingLastName),
            TEST(UsersTest::testCreateUserWhitespaceName),
            TEST(UsersTest::testCreateUserTrimsNames),
            TEST(UsersTest::testCreateUserDuplicateEmailCaseInsensitive),
            TEST(UsersTest::testGetUserSuccess),
            TEST(UsersTest::testGetUserNotFound),
            TEST(UsersTest::testGetUserInvalidID),
            TEST(UsersTest::testGetUserMissingID),
            TEST(UsersTest::testEditUserPartial),
            TEST(UsersTest::testEditUserEmailNormalization),
            TEST(UsersTest::testEditUserTrimsNames),
            TEST(UsersTest::testEditUserWhitespaceFirstName),
            TEST(UsersTest::testEditUserSameEmailCaseInsensitiveAllowed),
            TEST(UsersTest::testEditUserEmailConflict),
            TEST(UsersTest::testEditUserInvalidEmail),
            TEST(UsersTest::testEditUserInvalidID),
            TEST(UsersTest::testEditUserNotFound),
            TEST(UsersTest::testEditUserNoFields),
            TEST(UsersTest::testDeleteUserSuccess),
            TEST(UsersTest::testDeleteUserNotFound),
            TEST(UsersTest::testDeleteUserInvalidID),
            TEST(UsersTest::testDeleteUserMissingID),
            TEST(UsersTest::testDeleteUserAllowsEmailReuse),
            TEST(UsersTest::testDeleteUserCascadesCreatedPolls),
            TEST(UsersTest::testDeleteUserCascadesVotes),
            TEST(UsersTest::testDeleteUserCascadesMessages)
        ) { }

    string firstOptionID(BedrockTester& tester, const string& pollID) {
        return TestHelpers::firstOptionForPoll(tester, pollID).at("optionID");
    }

    void testCreateUserSuccess() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("CreateUser");
        req["email"] = "MAILTO:<TeSt.User+Tag@Example.com>";
        req["firstName"] = "First";
        req["lastName"] = "Last";
        req["displayName"] = "  Preferred Name  ";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "200 OK"));
        ASSERT_FALSE(resp["userID"].empty());
        ASSERT_EQUAL(resp["email"], "test.user+tag@example.com");
        ASSERT_EQUAL(resp["firstName"], "First");
        ASSERT_EQUAL(resp["lastName"], "Last");
        ASSERT_EQUAL(resp["displayName"], "Preferred Name");
        ASSERT_FALSE(resp["createdAt"].empty());
    }

    void testCreateUserMissingEmail() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("CreateUser");
        req["firstName"] = "First";
        req["lastName"] = "Last";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testCreateUserInvalidEmail() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("CreateUser");
        req["email"] = "not-an-email";
        req["firstName"] = "First";
        req["lastName"] = "Last";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testCreateUserMissingFirstName() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("CreateUser");
        req["email"] = "missingfirstname@example.com";
        req["lastName"] = "Last";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testCreateUserMissingLastName() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("CreateUser");
        req["email"] = "missinglastname@example.com";
        req["firstName"] = "First";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testCreateUserWhitespaceName() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("CreateUser");
        req["email"] = "blankname@example.com";
        req["firstName"] = "   ";
        req["lastName"] = "Last";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testCreateUserTrimsNames() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("CreateUser");
        req["email"] = "trim@example.com";
        req["firstName"] = "  First  ";
        req["lastName"] = "  Last  ";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "200 OK"));
        ASSERT_EQUAL(resp["firstName"], "First");
        ASSERT_EQUAL(resp["lastName"], "Last");
        ASSERT_EQUAL(resp["displayName"], "First Last");
    }

    void testCreateUserDuplicateEmailCaseInsensitive() {
        BedrockTester tester = TestHelpers::createTester();

        SData firstReq("CreateUser");
        firstReq["email"] = "person@example.com";
        firstReq["firstName"] = "One";
        firstReq["lastName"] = "User";
        SData firstResp = TestHelpers::executeSingle(tester, firstReq);
        ASSERT_TRUE(SStartsWith(firstResp.methodLine, "200 OK"));

        SData req("CreateUser");
        req["email"] = "PERSON@EXAMPLE.COM";
        req["firstName"] = "Two";
        req["lastName"] = "User";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "409"));
        ASSERT_EQUAL(resp["errorCode"], "CREATE_USER_EMAIL_CONFLICT");
        ASSERT_EQUAL(SParseJSONObject(resp.content).at("errorCode"), "CREATE_USER_EMAIL_CONFLICT");
    }

    void testGetUserSuccess() {
        BedrockTester tester = TestHelpers::createTester();
        const string userID = TestHelpers::createUserID(tester, "get", "Get", "User");

        SData req("GetUser");
        req["userID"] = userID;
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "200 OK"));
        ASSERT_EQUAL(resp["userID"], userID);
        ASSERT_EQUAL(resp["firstName"], "Get");
        ASSERT_EQUAL(resp["lastName"], "User");
        ASSERT_EQUAL(resp["displayName"], "Get User");
    }

    void testGetUserNotFound() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("GetUser");
        req["userID"] = "99999";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "404"));
    }

    void testGetUserInvalidID() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("GetUser");
        req["userID"] = "0";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testGetUserMissingID() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("GetUser");
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testEditUserPartial() {
        BedrockTester tester = TestHelpers::createTester();
        const string userID = TestHelpers::createUserID(tester, "edit", "Original", "User");

        SData editReq("EditUser");
        editReq["userID"] = userID;
        editReq["firstName"] = "Updated";
        editReq["displayName"] = "  Updated User  ";
        SData editResp = TestHelpers::executeSingle(tester, editReq);

        ASSERT_TRUE(SStartsWith(editResp.methodLine, "200 OK"));
        ASSERT_EQUAL(editResp["result"], "updated");
        ASSERT_EQUAL(editResp["firstName"], "Updated");
        ASSERT_EQUAL(editResp["lastName"], "User");
        ASSERT_EQUAL(editResp["displayName"], "Updated User");
    }

    void testEditUserEmailNormalization() {
        BedrockTester tester = TestHelpers::createTester();
        const string userID = TestHelpers::createUserID(tester, "old", "Old", "User");

        SData editReq("EditUser");
        editReq["userID"] = userID;
        editReq["email"] = "MAILTO:<NEW+Alias@Example.com>";
        SData editResp = TestHelpers::executeSingle(tester, editReq);

        ASSERT_TRUE(SStartsWith(editResp.methodLine, "200 OK"));
        ASSERT_EQUAL(editResp["email"], "new+alias@example.com");
    }

    void testEditUserTrimsNames() {
        BedrockTester tester = TestHelpers::createTester();
        const string userID = TestHelpers::createUserID(tester, "trim-edit", "Name", "Before");

        SData editReq("EditUser");
        editReq["userID"] = userID;
        editReq["firstName"] = "  After  ";
        editReq["lastName"] = "  Name  ";
        SData editResp = TestHelpers::executeSingle(tester, editReq);

        ASSERT_TRUE(SStartsWith(editResp.methodLine, "200 OK"));
        ASSERT_EQUAL(editResp["firstName"], "After");
        ASSERT_EQUAL(editResp["lastName"], "Name");
    }

    void testEditUserWhitespaceFirstName() {
        BedrockTester tester = TestHelpers::createTester();
        const string userID = TestHelpers::createUserID(tester, "invalid-edit", "Before", "Name");

        SData editReq("EditUser");
        editReq["userID"] = userID;
        editReq["firstName"] = "   ";
        SData editResp = TestHelpers::executeSingle(tester, editReq);

        ASSERT_TRUE(SStartsWith(editResp.methodLine, "400"));
    }

    void testEditUserSameEmailCaseInsensitiveAllowed() {
        BedrockTester tester = TestHelpers::createTester();

        SData createReq("CreateUser");
        createReq["email"] = "same@example.com";
        createReq["firstName"] = "Same";
        createReq["lastName"] = "User";
        SData createResp = TestHelpers::executeSingle(tester, createReq);
        ASSERT_TRUE(SStartsWith(createResp.methodLine, "200 OK"));
        const string userID = createResp["userID"];

        SData editReq("EditUser");
        editReq["userID"] = userID;
        editReq["email"] = "SAME@EXAMPLE.COM";
        SData editResp = TestHelpers::executeSingle(tester, editReq);

        ASSERT_TRUE(SStartsWith(editResp.methodLine, "200 OK"));
        ASSERT_EQUAL(editResp["email"], "same@example.com");
    }

    void testEditUserEmailConflict() {
        BedrockTester tester = TestHelpers::createTester();

        SData firstReq("CreateUser");
        firstReq["email"] = "primary@example.com";
        firstReq["firstName"] = "Primary";
        firstReq["lastName"] = "User";
        SData firstResp = TestHelpers::executeSingle(tester, firstReq);
        ASSERT_TRUE(SStartsWith(firstResp.methodLine, "200 OK"));

        SData secondReq("CreateUser");
        secondReq["email"] = "secondary@example.com";
        secondReq["firstName"] = "Secondary";
        secondReq["lastName"] = "User";
        SData secondResp = TestHelpers::executeSingle(tester, secondReq);
        ASSERT_TRUE(SStartsWith(secondResp.methodLine, "200 OK"));

        SData editReq("EditUser");
        editReq["userID"] = secondResp["userID"];
        editReq["email"] = "PRIMARY@EXAMPLE.COM";
        SData editResp = TestHelpers::executeSingle(tester, editReq);

        ASSERT_TRUE(SStartsWith(editResp.methodLine, "409"));
    }

    void testEditUserInvalidEmail() {
        BedrockTester tester = TestHelpers::createTester();
        const string userID = TestHelpers::createUserID(tester, "valid", "Valid", "User");

        SData editReq("EditUser");
        editReq["userID"] = userID;
        editReq["email"] = "not-an-email";
        SData editResp = TestHelpers::executeSingle(tester, editReq);

        ASSERT_TRUE(SStartsWith(editResp.methodLine, "400"));
    }

    void testEditUserInvalidID() {
        BedrockTester tester = TestHelpers::createTester();

        SData editReq("EditUser");
        editReq["userID"] = "0";
        editReq["firstName"] = "Nope";
        SData editResp = TestHelpers::executeSingle(tester, editReq);

        ASSERT_TRUE(SStartsWith(editResp.methodLine, "400"));
    }

    void testEditUserNotFound() {
        BedrockTester tester = TestHelpers::createTester();

        SData editReq("EditUser");
        editReq["userID"] = "99999";
        editReq["firstName"] = "Nope";
        SData editResp = TestHelpers::executeSingle(tester, editReq);

        ASSERT_TRUE(SStartsWith(editResp.methodLine, "404"));
    }

    void testEditUserNoFields() {
        BedrockTester tester = TestHelpers::createTester();
        const string userID = TestHelpers::createUserID(tester, "nofields", "No", "Fields");

        SData editReq("EditUser");
        editReq["userID"] = userID;
        SData editResp = TestHelpers::executeSingle(tester, editReq);

        ASSERT_TRUE(SStartsWith(editResp.methodLine, "400"));
    }

    void testDeleteUserSuccess() {
        BedrockTester tester = TestHelpers::createTester();
        const string userID = TestHelpers::createUserID(tester, "delete", "Delete", "Me");

        SData deleteReq("DeleteUser");
        deleteReq["userID"] = userID;
        SData deleteResp = TestHelpers::executeSingle(tester, deleteReq);

        ASSERT_TRUE(SStartsWith(deleteResp.methodLine, "200 OK"));
        ASSERT_EQUAL(deleteResp["result"], "deleted");

        SData getReq("GetUser");
        getReq["userID"] = userID;
        SData getResp = TestHelpers::executeSingle(tester, getReq);
        ASSERT_TRUE(SStartsWith(getResp.methodLine, "404"));
    }

    void testDeleteUserNotFound() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("DeleteUser");
        req["userID"] = "99999";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "404"));
    }

    void testDeleteUserInvalidID() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("DeleteUser");
        req["userID"] = "0";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testDeleteUserMissingID() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("DeleteUser");
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testDeleteUserAllowsEmailReuse() {
        BedrockTester tester = TestHelpers::createTester();

        SData createReq("CreateUser");
        createReq["email"] = "reuse@example.com";
        createReq["firstName"] = "Reuse";
        createReq["lastName"] = "One";
        SData createResp = TestHelpers::executeSingle(tester, createReq);
        ASSERT_TRUE(SStartsWith(createResp.methodLine, "200 OK"));

        SData deleteReq("DeleteUser");
        deleteReq["userID"] = createResp["userID"];
        SData deleteResp = TestHelpers::executeSingle(tester, deleteReq);
        ASSERT_TRUE(SStartsWith(deleteResp.methodLine, "200 OK"));

        SData recreateReq("CreateUser");
        recreateReq["email"] = "reuse@example.com";
        recreateReq["firstName"] = "Reuse";
        recreateReq["lastName"] = "Two";
        SData recreateResp = TestHelpers::executeSingle(tester, recreateReq);

        ASSERT_TRUE(SStartsWith(recreateResp.methodLine, "200 OK"));
    }

    void testDeleteUserCascadesCreatedPolls() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "creator", "Creator", "User");
        const string pollID = TestHelpers::createPollID(tester, creatorID, "Owned poll", "[\"A\",\"B\"]");

        SData deleteReq("DeleteUser");
        deleteReq["userID"] = creatorID;
        SData deleteResp = TestHelpers::executeSingle(tester, deleteReq);
        ASSERT_TRUE(SStartsWith(deleteResp.methodLine, "200 OK"));

        ASSERT_EQUAL(
            tester.readDB("SELECT COUNT(*) FROM polls WHERE pollID = " + pollID + ";"),
            "0"
        );
    }

    void testDeleteUserCascadesVotes() {
        BedrockTester tester = TestHelpers::createTester();
        const string creatorID = TestHelpers::createUserID(tester, "creatorv", "Creator", "Votes");
        const string voter1ID = TestHelpers::createUserID(tester, "voter1", "Vote", "One");
        const string voter2ID = TestHelpers::createUserID(tester, "voter2", "Vote", "Two");
        const string pollID = TestHelpers::createPollID(tester, creatorID, "Vote cascade", "[\"A\",\"B\"]");
        const string optionID = firstOptionID(tester, pollID);

        SData voteResp1 = TestHelpers::submitVote(tester, pollID, optionID, voter1ID);
        ASSERT_TRUE(SStartsWith(voteResp1.methodLine, "200 OK"));

        SData voteResp2 = TestHelpers::submitVote(tester, pollID, optionID, voter2ID);
        ASSERT_TRUE(SStartsWith(voteResp2.methodLine, "200 OK"));

        SData deleteReq("DeleteUser");
        deleteReq["userID"] = voter1ID;
        SData deleteResp = TestHelpers::executeSingle(tester, deleteReq);
        ASSERT_TRUE(SStartsWith(deleteResp.methodLine, "200 OK"));

        SData getPollResp = TestHelpers::getPoll(tester, pollID, creatorID);
        ASSERT_TRUE(SStartsWith(getPollResp.methodLine, "200 OK"));
        ASSERT_EQUAL(getPollResp["totalVotes"], "1");
    }

    void testDeleteUserCascadesMessages() {
        BedrockTester tester = TestHelpers::createTester();
        const string ownerID = TestHelpers::createUserID(tester, "msg-owner", "Message", "Owner");
        const string userID = TestHelpers::createUserID(tester, "msg", "Message", "Member");
        const string chatID = TestHelpers::createChatID(tester, ownerID, "Delete user messages");
        TestHelpers::addChatMember(tester, chatID, ownerID, userID, "member");
        const string messageID = TestHelpers::createChatMessageID(tester, chatID, userID, "Owned message");

        SData deleteReq("DeleteUser");
        deleteReq["userID"] = userID;
        SData deleteResp = TestHelpers::executeSingle(tester, deleteReq);
        ASSERT_TRUE(SStartsWith(deleteResp.methodLine, "200 OK"));

        SData listReq("GetChatMessages");
        listReq["chatID"] = chatID;
        listReq["userID"] = ownerID;
        listReq["limit"] = "50";
        SData listResp = TestHelpers::executeSingle(tester, listReq);
        ASSERT_TRUE(SStartsWith(listResp.methodLine, "200 OK"));

        list<string> rows = SParseJSONArray(listResp["messages"]);
        bool found = false;
        for (const string& row : rows) {
            const STable entry = SParseJSONObject(row);
            if (entry.at("messageID") == messageID) {
                found = true;
                break;
            }
        }

        ASSERT_FALSE(found);
    }
};
