#pragma once

#include "../../../TestHelpers.h"

struct ChatMessagesTest : tpunit::TestFixture {
    ChatMessagesTest()
        : tpunit::TestFixture(
            "ChatMessagesTests",
            TEST(ChatMessagesTest::testCreateAndGet),
            TEST(ChatMessagesTest::testCreateChatMessageRequiresMembership),
            TEST(ChatMessagesTest::testCreateChatMessageMissingBody),
            TEST(ChatMessagesTest::testGetMessagesForbiddenForNonMember),
            TEST(ChatMessagesTest::testGetMessagesDefaultLimit),
            TEST(ChatMessagesTest::testGetMessagesLimitBounds),
            TEST(ChatMessagesTest::testGetMessagesDescendingOrder),
            TEST(ChatMessagesTest::testGetMessagesPaginationBeforeMessageID),
            TEST(ChatMessagesTest::testEditChatMessageAuthorAndOwner),
            TEST(ChatMessagesTest::testEditChatMessageRejectsBlankBody),
            TEST(ChatMessagesTest::testEditChatMessageNotFound),
            TEST(ChatMessagesTest::testDeleteChatMessageAuthorAndOwner),
            TEST(ChatMessagesTest::testDeleteChatMessageNotFound)
        ) { }

    string createOwnerChat(BedrockTester& tester, string& ownerID) {
        ownerID = TestHelpers::createUserID(tester, "messages-owner", "Message", "Owner");
        return TestHelpers::createChatID(tester, ownerID, "Message chat");
    }

    void testCreateAndGet() {
        BedrockTester tester = TestHelpers::createTester();
        string ownerID;
        const string chatID = createOwnerChat(tester, ownerID);
        const string messageText = "Integration test " + SToStr(STimeNow());

        SData createRequest("CreateChatMessage");
        createRequest["chatID"] = chatID;
        createRequest["userID"] = ownerID;
        createRequest["body"] = messageText;
        SData createResponse = TestHelpers::executeSingle(tester, createRequest);

        ASSERT_TRUE(SStartsWith(createResponse.methodLine, "200 OK"));
        ASSERT_EQUAL(createResponse["result"], "stored");
        ASSERT_EQUAL(createResponse["chatID"], chatID);
        ASSERT_EQUAL(createResponse["userID"], ownerID);
        ASSERT_EQUAL(createResponse["body"], messageText);
        ASSERT_FALSE(createResponse["createdAt"].empty());
        ASSERT_FALSE(createResponse["updatedAt"].empty());
        const string messageID = createResponse["messageID"];
        ASSERT_FALSE(messageID.empty());

        SData listRequest("GetChatMessages");
        listRequest["chatID"] = chatID;
        listRequest["userID"] = ownerID;
        listRequest["limit"] = "10";
        SData listResponse = TestHelpers::executeSingle(tester, listRequest);

        ASSERT_TRUE(SStartsWith(listResponse.methodLine, "200 OK"));
        ASSERT_FALSE(listResponse["messages"].empty());

        list<string> rows = SParseJSONArray(listResponse["messages"]);
        bool found = false;
        for (const string& row : rows) {
            const STable entry = SParseJSONObject(row);
            if (entry.at("messageID") == messageID) {
                ASSERT_EQUAL(entry.at("chatID"), chatID);
                ASSERT_EQUAL(entry.at("userID"), ownerID);
                ASSERT_EQUAL(entry.at("body"), messageText);
                ASSERT_FALSE(entry.at("createdAt").empty());
                ASSERT_FALSE(entry.at("updatedAt").empty());
                found = true;
                break;
            }
        }

        ASSERT_TRUE(found);
    }

    void testCreateChatMessageRequiresMembership() {
        BedrockTester tester = TestHelpers::createTester();
        const string ownerID = TestHelpers::createUserID(tester, "owner", "Owner", "User");
        const string outsiderID = TestHelpers::createUserID(tester, "outsider", "Out", "Sider");
        const string chatID = TestHelpers::createChatID(tester, ownerID, "Restricted");

        SData req("CreateChatMessage");
        req["chatID"] = chatID;
        req["userID"] = outsiderID;
        req["body"] = "Should fail";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "403"));
    }

    void testCreateChatMessageMissingBody() {
        BedrockTester tester = TestHelpers::createTester();
        string ownerID;
        const string chatID = createOwnerChat(tester, ownerID);

        SData req("CreateChatMessage");
        req["chatID"] = chatID;
        req["userID"] = ownerID;
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testGetMessagesForbiddenForNonMember() {
        BedrockTester tester = TestHelpers::createTester();
        const string ownerID = TestHelpers::createUserID(tester, "owner", "Owner", "User");
        const string outsiderID = TestHelpers::createUserID(tester, "outsider", "Out", "Sider");
        const string chatID = TestHelpers::createChatID(tester, ownerID, "Restricted");

        (void)TestHelpers::createChatMessageID(tester, chatID, ownerID, "Owner message");

        SData req("GetChatMessages");
        req["chatID"] = chatID;
        req["userID"] = outsiderID;
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "403"));
    }

    void testGetMessagesDefaultLimit() {
        BedrockTester tester = TestHelpers::createTester();
        string ownerID;
        const string chatID = createOwnerChat(tester, ownerID);

        for (int i = 0; i < 25; i++) {
            (void)TestHelpers::createChatMessageID(tester, chatID, ownerID, "Message " + SToStr(i));
        }

        SData req("GetChatMessages");
        req["chatID"] = chatID;
        req["userID"] = ownerID;
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "200 OK"));
        ASSERT_EQUAL(resp["resultCount"], "20");

        list<string> rows = SParseJSONArray(resp["messages"]);
        ASSERT_EQUAL(rows.size(), static_cast<size_t>(20));
    }

    void testGetMessagesLimitBounds() {
        BedrockTester tester = TestHelpers::createTester();
        string ownerID;
        const string chatID = createOwnerChat(tester, ownerID);
        (void)TestHelpers::createChatMessageID(tester, chatID, ownerID, "Bound checks");

        SData lowReq("GetChatMessages");
        lowReq["chatID"] = chatID;
        lowReq["userID"] = ownerID;
        lowReq["limit"] = "0";
        SData lowResp = TestHelpers::executeSingle(tester, lowReq);
        ASSERT_TRUE(SStartsWith(lowResp.methodLine, "400"));

        SData highReq("GetChatMessages");
        highReq["chatID"] = chatID;
        highReq["userID"] = ownerID;
        highReq["limit"] = "101";
        SData highResp = TestHelpers::executeSingle(tester, highReq);
        ASSERT_TRUE(SStartsWith(highResp.methodLine, "400"));
    }

    void testGetMessagesDescendingOrder() {
        BedrockTester tester = TestHelpers::createTester();
        string ownerID;
        const string chatID = createOwnerChat(tester, ownerID);

        const string firstID = TestHelpers::createChatMessageID(tester, chatID, ownerID, "first");
        const string secondID = TestHelpers::createChatMessageID(tester, chatID, ownerID, "second");

        SData req("GetChatMessages");
        req["chatID"] = chatID;
        req["userID"] = ownerID;
        req["limit"] = "2";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "200 OK"));

        list<string> rows = SParseJSONArray(resp["messages"]);
        ASSERT_EQUAL(rows.size(), static_cast<size_t>(2));

        auto it = rows.begin();
        const STable newest = SParseJSONObject(*it++);
        const STable older = SParseJSONObject(*it);
        ASSERT_EQUAL(newest.at("messageID"), secondID);
        ASSERT_EQUAL(older.at("messageID"), firstID);
    }

    void testGetMessagesPaginationBeforeMessageID() {
        BedrockTester tester = TestHelpers::createTester();
        string ownerID;
        const string chatID = createOwnerChat(tester, ownerID);

        (void)TestHelpers::createChatMessageID(tester, chatID, ownerID, "m1");
        const string middleID = TestHelpers::createChatMessageID(tester, chatID, ownerID, "m2");
        const string newestID = TestHelpers::createChatMessageID(tester, chatID, ownerID, "m3");

        SData req("GetChatMessages");
        req["chatID"] = chatID;
        req["userID"] = ownerID;
        req["beforeMessageID"] = newestID;
        req["limit"] = "2";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "200 OK"));

        list<string> rows = SParseJSONArray(resp["messages"]);
        ASSERT_FALSE(rows.empty());

        const STable first = SParseJSONObject(rows.front());
        ASSERT_EQUAL(first.at("messageID"), middleID);
    }

    void testEditChatMessageAuthorAndOwner() {
        BedrockTester tester = TestHelpers::createTester();
        const string ownerID = TestHelpers::createUserID(tester, "owner", "Owner", "User");
        const string authorID = TestHelpers::createUserID(tester, "author", "Author", "User");
        const string outsiderID = TestHelpers::createUserID(tester, "outsider", "Out", "Sider");
        const string chatID = TestHelpers::createChatID(tester, ownerID, "Moderation");

        TestHelpers::addChatMember(tester, chatID, ownerID, authorID, "member");

        const string firstMessageID = TestHelpers::createChatMessageID(tester, chatID, authorID, "original1");
        const string secondMessageID = TestHelpers::createChatMessageID(tester, chatID, authorID, "original2");

        SData forbiddenReq("EditChatMessage");
        forbiddenReq["chatID"] = chatID;
        forbiddenReq["messageID"] = firstMessageID;
        forbiddenReq["userID"] = outsiderID;
        forbiddenReq["body"] = "forbidden";
        SData forbiddenResp = TestHelpers::executeSingle(tester, forbiddenReq);
        ASSERT_TRUE(SStartsWith(forbiddenResp.methodLine, "403"));

        SData authorReq("EditChatMessage");
        authorReq["chatID"] = chatID;
        authorReq["messageID"] = firstMessageID;
        authorReq["userID"] = authorID;
        authorReq["body"] = "author edit";
        SData authorResp = TestHelpers::executeSingle(tester, authorReq);
        ASSERT_TRUE(SStartsWith(authorResp.methodLine, "200 OK"));
        ASSERT_EQUAL(authorResp["body"], "author edit");

        SData ownerReq("EditChatMessage");
        ownerReq["chatID"] = chatID;
        ownerReq["messageID"] = secondMessageID;
        ownerReq["userID"] = ownerID;
        ownerReq["body"] = "owner edit";
        SData ownerResp = TestHelpers::executeSingle(tester, ownerReq);

        ASSERT_TRUE(SStartsWith(ownerResp.methodLine, "200 OK"));
        ASSERT_EQUAL(ownerResp["body"], "owner edit");
    }

    void testEditChatMessageNotFound() {
        BedrockTester tester = TestHelpers::createTester();
        string ownerID;
        const string chatID = createOwnerChat(tester, ownerID);

        SData req("EditChatMessage");
        req["chatID"] = chatID;
        req["messageID"] = "99999";
        req["userID"] = ownerID;
        req["body"] = "updated";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "404"));
    }

    void testEditChatMessageRejectsBlankBody() {
        BedrockTester tester = TestHelpers::createTester();
        string ownerID;
        const string chatID = createOwnerChat(tester, ownerID);
        const string messageID = TestHelpers::createChatMessageID(tester, chatID, ownerID, "original");

        SData req("EditChatMessage");
        req["chatID"] = chatID;
        req["messageID"] = messageID;
        req["userID"] = ownerID;
        req["body"] = "   ";
        const SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testDeleteChatMessageAuthorAndOwner() {
        BedrockTester tester = TestHelpers::createTester();
        const string ownerID = TestHelpers::createUserID(tester, "owner", "Owner", "User");
        const string authorID = TestHelpers::createUserID(tester, "author", "Author", "User");
        const string outsiderID = TestHelpers::createUserID(tester, "outsider", "Out", "Sider");
        const string chatID = TestHelpers::createChatID(tester, ownerID, "Moderation");

        TestHelpers::addChatMember(tester, chatID, ownerID, authorID, "member");

        const string firstMessageID = TestHelpers::createChatMessageID(tester, chatID, authorID, "first");
        const string secondMessageID = TestHelpers::createChatMessageID(tester, chatID, authorID, "second");

        SData forbiddenReq("DeleteChatMessage");
        forbiddenReq["chatID"] = chatID;
        forbiddenReq["messageID"] = firstMessageID;
        forbiddenReq["userID"] = outsiderID;
        SData forbiddenResp = TestHelpers::executeSingle(tester, forbiddenReq);
        ASSERT_TRUE(SStartsWith(forbiddenResp.methodLine, "403"));

        SData ownerReq("DeleteChatMessage");
        ownerReq["chatID"] = chatID;
        ownerReq["messageID"] = firstMessageID;
        ownerReq["userID"] = ownerID;
        SData ownerResp = TestHelpers::executeSingle(tester, ownerReq);
        ASSERT_TRUE(SStartsWith(ownerResp.methodLine, "200 OK"));

        SData authorReq("DeleteChatMessage");
        authorReq["chatID"] = chatID;
        authorReq["messageID"] = secondMessageID;
        authorReq["userID"] = authorID;
        SData authorResp = TestHelpers::executeSingle(tester, authorReq);
        ASSERT_TRUE(SStartsWith(authorResp.methodLine, "200 OK"));
    }

    void testDeleteChatMessageNotFound() {
        BedrockTester tester = TestHelpers::createTester();
        string ownerID;
        const string chatID = createOwnerChat(tester, ownerID);

        SData req("DeleteChatMessage");
        req["chatID"] = chatID;
        req["messageID"] = "99999";
        req["userID"] = ownerID;
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "404"));
    }
};
