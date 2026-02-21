#pragma once

#include "../TestHelpers.h"
#include <libstuff/SData.h>

struct MessagesTest : tpunit::TestFixture {
    MessagesTest()
        : tpunit::TestFixture(
            "MessagesTests",
            TEST(MessagesTest::testCreateAndGet),
            TEST(MessagesTest::testCreateMessageMissingUserID),
            TEST(MessagesTest::testCreateMessageInvalidUserIDFormat),
            TEST(MessagesTest::testCreateMessageUnknownUserID),
            TEST(MessagesTest::testCreateMessageMissingName),
            TEST(MessagesTest::testCreateMessageMissingMessage),
            TEST(MessagesTest::testGetMessagesDefaultLimit),
            TEST(MessagesTest::testGetMessagesLimitBounds),
            TEST(MessagesTest::testGetMessagesLimitInvalidFormat),
            TEST(MessagesTest::testGetMessagesDescendingOrder)
        ) { }

    void testCreateAndGet() {
        BedrockTester tester = TestHelpers::createTester();
        const string userID = TestHelpers::createUserID(tester, "messages", "Message", "Tester");
        const string messageText = "Integration test " + SToStr(STimeNow());

        SData createRequest("CreateMessage");
        createRequest["userID"] = userID;
        createRequest["name"] = "Tester";
        createRequest["message"] = messageText;
        SData createResponse = TestHelpers::executeSingle(tester, createRequest);

        ASSERT_TRUE(SStartsWith(createResponse.methodLine, "200 OK"));
        ASSERT_EQUAL(createResponse["result"], "stored");
        ASSERT_EQUAL(createResponse["userID"], userID);
        ASSERT_EQUAL(createResponse["message"], messageText);
        ASSERT_FALSE(createResponse["createdAt"].empty());
        const string messageID = createResponse["messageID"];
        ASSERT_FALSE(messageID.empty());

        SData listRequest("GetMessages");
        listRequest["limit"] = "10";
        SData listResponse = TestHelpers::executeSingle(tester, listRequest);

        ASSERT_TRUE(SStartsWith(listResponse.methodLine, "200 OK"));
        ASSERT_FALSE(listResponse["messages"].empty());

        list<string> rows = SParseJSONArray(listResponse["messages"]);
        bool found = false;
        for (const string& row : rows) {
            const STable entry = SParseJSONObject(row);
            if (entry.at("messageID") == messageID) {
                ASSERT_EQUAL(entry.at("userID"), userID);
                ASSERT_EQUAL(entry.at("name"), "Tester");
                ASSERT_EQUAL(entry.at("message"), messageText);
                ASSERT_FALSE(entry.at("createdAt").empty());
                found = true;
                break;
            }
        }

        ASSERT_TRUE(found);
    }

    void testCreateMessageMissingUserID() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("CreateMessage");
        req["name"] = "NoUser";
        req["message"] = "Body";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testCreateMessageInvalidUserIDFormat() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("CreateMessage");
        req["userID"] = "abc";
        req["name"] = "NoUser";
        req["message"] = "Body";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
        ASSERT_EQUAL(resp["errorCode"], "INVALID_PARAMETER");
        ASSERT_FALSE(resp.content.empty());
        ASSERT_EQUAL(SParseJSONObject(resp.content).at("errorCode"), "INVALID_PARAMETER");
    }

    void testCreateMessageUnknownUserID() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("CreateMessage");
        req["userID"] = "99999";
        req["name"] = "NoUser";
        req["message"] = "Body";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "404"));
    }

    void testCreateMessageMissingName() {
        BedrockTester tester = TestHelpers::createTester();
        const string userID = TestHelpers::createUserID(tester, "messages");

        SData req("CreateMessage");
        req["userID"] = userID;
        req["message"] = "Body only";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testCreateMessageMissingMessage() {
        BedrockTester tester = TestHelpers::createTester();
        const string userID = TestHelpers::createUserID(tester, "messages");

        SData req("CreateMessage");
        req["userID"] = userID;
        req["name"] = "OnlyName";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testGetMessagesDefaultLimit() {
        BedrockTester tester = TestHelpers::createTester();
        const string userID = TestHelpers::createUserID(tester, "messages");

        for (int i = 0; i < 25; i++) {
            (void)TestHelpers::createMessageID(tester, userID, "Tester", "Message " + SToStr(i));
        }

        SData req("GetMessages");
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "200 OK"));
        ASSERT_EQUAL(resp["resultCount"], "20");

        list<string> rows = SParseJSONArray(resp["messages"]);
        ASSERT_EQUAL(rows.size(), static_cast<size_t>(20));
    }

    void testGetMessagesLimitBounds() {
        BedrockTester tester = TestHelpers::createTester();
        const string userID = TestHelpers::createUserID(tester, "messages");
        (void)TestHelpers::createMessageID(tester, userID, "Tester", "Bound checks");

        SData lowReq("GetMessages");
        lowReq["limit"] = "0";
        SData lowResp = TestHelpers::executeSingle(tester, lowReq);
        ASSERT_TRUE(SStartsWith(lowResp.methodLine, "400"));

        SData highReq("GetMessages");
        highReq["limit"] = "101";
        SData highResp = TestHelpers::executeSingle(tester, highReq);
        ASSERT_TRUE(SStartsWith(highResp.methodLine, "400"));
    }

    void testGetMessagesLimitInvalidFormat() {
        BedrockTester tester = TestHelpers::createTester();

        SData req("GetMessages");
        req["limit"] = "ten";
        SData resp = TestHelpers::executeSingle(tester, req);

        ASSERT_TRUE(SStartsWith(resp.methodLine, "400"));
    }

    void testGetMessagesDescendingOrder() {
        BedrockTester tester = TestHelpers::createTester();
        const string userID = TestHelpers::createUserID(tester, "messages");

        const string firstID = TestHelpers::createMessageID(tester, userID, "Tester", "first");
        const string secondID = TestHelpers::createMessageID(tester, userID, "Tester", "second");

        SData req("GetMessages");
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
};
