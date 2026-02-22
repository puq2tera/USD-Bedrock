#pragma once

#include "../../../TestHelpers.h"

struct ChatsTest : tpunit::TestFixture {
    ChatsTest()
        : tpunit::TestFixture(
            "ChatsTests",
            TEST(ChatsTest::testCreateGetListEditDeleteChat),
            TEST(ChatsTest::testCreateChatCreatorNotFound),
            TEST(ChatsTest::testCreateChatRejectsBlankTitle),
            TEST(ChatsTest::testGetChatForbiddenForNonMember),
            TEST(ChatsTest::testEditChatRejectsBlankTitle),
            TEST(ChatsTest::testListChatsPaginationAndLimit),
            TEST(ChatsTest::testEditChatForbiddenForMember),
            TEST(ChatsTest::testDeleteChatForbiddenForMember),
            TEST(ChatsTest::testDeleteChatNotFound)
        ) { }

    void testCreateGetListEditDeleteChat() {
        BedrockTester tester = TestHelpers::createTester();
        const string ownerID = TestHelpers::createUserID(tester, "chat-owner", "Chat", "Owner");

        SData createReq("CreateChat");
        createReq["creatorUserID"] = ownerID;
        createReq["title"] = "Engineering";
        SData createResp = TestHelpers::executeSingle(tester, createReq);

        ASSERT_TRUE(SStartsWith(createResp.methodLine, "200 OK"));
        const string chatID = createResp["chatID"];
        ASSERT_FALSE(chatID.empty());
        ASSERT_EQUAL(createResp["createdByUserID"], ownerID);
        ASSERT_EQUAL(createResp["title"], "Engineering");

        SData getReq("GetChat");
        getReq["chatID"] = chatID;
        getReq["userID"] = ownerID;
        SData getResp = TestHelpers::executeSingle(tester, getReq);

        ASSERT_TRUE(SStartsWith(getResp.methodLine, "200 OK"));
        ASSERT_EQUAL(getResp["chatID"], chatID);
        ASSERT_EQUAL(getResp["requesterRole"], "owner");

        SData listReq("ListChats");
        listReq["userID"] = ownerID;
        SData listResp = TestHelpers::executeSingle(tester, listReq);

        ASSERT_TRUE(SStartsWith(listResp.methodLine, "200 OK"));
        list<string> chats = SParseJSONArray(listResp["chats"]);
        ASSERT_FALSE(chats.empty());

        bool found = false;
        for (const string& chatRow : chats) {
            const STable chat = SParseJSONObject(chatRow);
            if (chat.at("chatID") == chatID) {
                ASSERT_EQUAL(chat.at("requesterRole"), "owner");
                found = true;
                break;
            }
        }
        ASSERT_TRUE(found);

        SData editReq("EditChat");
        editReq["chatID"] = chatID;
        editReq["userID"] = ownerID;
        editReq["title"] = "Product";
        SData editResp = TestHelpers::executeSingle(tester, editReq);

        ASSERT_TRUE(SStartsWith(editResp.methodLine, "200 OK"));
        ASSERT_EQUAL(editResp["title"], "Product");

        SData deleteReq("DeleteChat");
        deleteReq["chatID"] = chatID;
        deleteReq["userID"] = ownerID;
        SData deleteResp = TestHelpers::executeSingle(tester, deleteReq);

        ASSERT_TRUE(SStartsWith(deleteResp.methodLine, "200 OK"));

        SData getAfterDeleteReq("GetChat");
        getAfterDeleteReq["chatID"] = chatID;
        getAfterDeleteReq["userID"] = ownerID;
        SData getAfterDeleteResp = TestHelpers::executeSingle(tester, getAfterDeleteReq);

        ASSERT_TRUE(SStartsWith(getAfterDeleteResp.methodLine, "404"));
    }

    void testCreateChatCreatorNotFound() {
        BedrockTester tester = TestHelpers::createTester();

        SData createReq("CreateChat");
        createReq["creatorUserID"] = "999999";
        createReq["title"] = "Missing creator";
        SData createResp = TestHelpers::executeSingle(tester, createReq);

        ASSERT_TRUE(SStartsWith(createResp.methodLine, "404"));
    }

    void testCreateChatRejectsBlankTitle() {
        BedrockTester tester = TestHelpers::createTester();
        const string ownerID = TestHelpers::createUserID(tester, "chat-blank-title", "Chat", "Owner");

        SData createReq("CreateChat");
        createReq["creatorUserID"] = ownerID;
        createReq["title"] = "   ";
        SData createResp = TestHelpers::executeSingle(tester, createReq);

        ASSERT_TRUE(SStartsWith(createResp.methodLine, "400"));
    }

    void testGetChatForbiddenForNonMember() {
        BedrockTester tester = TestHelpers::createTester();
        const string ownerID = TestHelpers::createUserID(tester, "owner", "Owner", "User");
        const string outsiderID = TestHelpers::createUserID(tester, "outsider", "Out", "Sider");
        const string chatID = TestHelpers::createChatID(tester, ownerID, "Private chat");

        SData req("GetChat");
        req["chatID"] = chatID;
        req["userID"] = outsiderID;
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "403"));
    }

    void testEditChatRejectsBlankTitle() {
        BedrockTester tester = TestHelpers::createTester();
        const string ownerID = TestHelpers::createUserID(tester, "owner", "Owner", "User");
        const string chatID = TestHelpers::createChatID(tester, ownerID, "Original title");

        SData editReq("EditChat");
        editReq["chatID"] = chatID;
        editReq["userID"] = ownerID;
        editReq["title"] = "    ";
        const SData editResp = TestHelpers::executeSingle(tester, editReq);

        ASSERT_TRUE(SStartsWith(editResp.methodLine, "400"));
    }

    void testListChatsPaginationAndLimit() {
        BedrockTester tester = TestHelpers::createTester();
        const string ownerID = TestHelpers::createUserID(tester, "chat-list", "Chat", "Lister");

        const string chatA = TestHelpers::createChatID(tester, ownerID, "Chat A");
        const string chatB = TestHelpers::createChatID(tester, ownerID, "Chat B");
        const string chatC = TestHelpers::createChatID(tester, ownerID, "Chat C");

        SData firstPageReq("ListChats");
        firstPageReq["userID"] = ownerID;
        firstPageReq["limit"] = "2";
        SData firstPageResp = TestHelpers::executeSingle(tester, firstPageReq);
        ASSERT_TRUE(SStartsWith(firstPageResp.methodLine, "200 OK"));

        list<string> firstPageRows = SParseJSONArray(firstPageResp["chats"]);
        ASSERT_EQUAL(firstPageRows.size(), static_cast<size_t>(2));

        const STable newest = SParseJSONObject(firstPageRows.front());
        const STable olderInFirstPage = SParseJSONObject(*next(firstPageRows.begin()));
        ASSERT_EQUAL(newest.at("chatID"), chatC);
        ASSERT_EQUAL(olderInFirstPage.at("chatID"), chatB);

        SData secondPageReq("ListChats");
        secondPageReq["userID"] = ownerID;
        secondPageReq["beforeChatID"] = olderInFirstPage.at("chatID");
        secondPageReq["limit"] = "2";
        SData secondPageResp = TestHelpers::executeSingle(tester, secondPageReq);
        ASSERT_TRUE(SStartsWith(secondPageResp.methodLine, "200 OK"));

        list<string> secondPageRows = SParseJSONArray(secondPageResp["chats"]);
        ASSERT_EQUAL(secondPageRows.size(), static_cast<size_t>(1));
        ASSERT_EQUAL(SParseJSONObject(secondPageRows.front()).at("chatID"), chatA);
    }

    void testEditChatForbiddenForMember() {
        BedrockTester tester = TestHelpers::createTester();
        const string ownerID = TestHelpers::createUserID(tester, "owner", "Owner", "User");
        const string memberID = TestHelpers::createUserID(tester, "member", "Member", "User");
        const string chatID = TestHelpers::createChatID(tester, ownerID, "Team chat");

        TestHelpers::addChatMember(tester, chatID, ownerID, memberID, "member");

        SData editReq("EditChat");
        editReq["chatID"] = chatID;
        editReq["userID"] = memberID;
        editReq["title"] = "Attempted rename";
        SData editResp = TestHelpers::executeSingle(tester, editReq);

        ASSERT_TRUE(SStartsWith(editResp.methodLine, "403"));
    }

    void testDeleteChatForbiddenForMember() {
        BedrockTester tester = TestHelpers::createTester();
        const string ownerID = TestHelpers::createUserID(tester, "owner", "Owner", "User");
        const string memberID = TestHelpers::createUserID(tester, "member", "Member", "User");
        const string chatID = TestHelpers::createChatID(tester, ownerID, "Team chat");

        TestHelpers::addChatMember(tester, chatID, ownerID, memberID, "member");

        SData deleteReq("DeleteChat");
        deleteReq["chatID"] = chatID;
        deleteReq["userID"] = memberID;
        SData deleteResp = TestHelpers::executeSingle(tester, deleteReq);

        ASSERT_TRUE(SStartsWith(deleteResp.methodLine, "403"));
    }

    void testDeleteChatNotFound() {
        BedrockTester tester = TestHelpers::createTester();
        const string ownerID = TestHelpers::createUserID(tester, "owner", "Owner", "User");

        SData deleteReq("DeleteChat");
        deleteReq["chatID"] = "99999";
        deleteReq["userID"] = ownerID;
        SData deleteResp = TestHelpers::executeSingle(tester, deleteReq);

        ASSERT_TRUE(SStartsWith(deleteResp.methodLine, "404"));
    }
};
