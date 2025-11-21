#pragma once

#include <libstuff/libstuff.h>
#include <test/lib/BedrockTester.h>
#include <test/lib/tpunit++.hpp>

#include <map>
#include <vector>
#include <list>

#ifndef CORE_TEST_PLUGIN_DIR
#    error "CORE_TEST_PLUGIN_DIR must be defined"
#endif

#ifndef CORE_TEST_BEDROCK_BIN
#    error "CORE_TEST_BEDROCK_BIN must be defined"
#endif

struct CoreTester : tpunit::TestFixture {
    CoreTester()
        : tpunit::TestFixture(
            "CoreTests",
            TEST(CoreTester::Core_HelloWorld),
            TEST(CoreTester::Core_MessagesCRUD)
        ) { }

    void Core_HelloWorld() {
        const string corePluginPath = string(CORE_TEST_PLUGIN_DIR) + "/Core.so";
        map<string, string> args = {
            {"-plugins", "DB," + corePluginPath},
            {"-db", BedrockTester::getTempFileName("coretest")}
        };

        BedrockTester tester(args, {}, 0, 0, 0, true, CORE_TEST_BEDROCK_BIN);

        SData request("HelloWorld");
        request["name"] = "TestUser";
        vector<SData> responses = tester.executeWaitMultipleData({request}, 1);
        SData response = responses.front();
        ASSERT_TRUE(SStartsWith(response.methodLine, "200 OK"));
        ASSERT_EQUAL(response["message"], "Hello, TestUser!");
        ASSERT_EQUAL(response["from"], "Bedrock Core Plugin");
        ASSERT_EQUAL(response["plugin_name"], "Core");
        ASSERT_EQUAL(response["plugin_version"], "1.1.0");

        SData request2("HelloWorld");
        vector<SData> responses2 = tester.executeWaitMultipleData({request2}, 1);
        SData response2 = responses2.front();
        ASSERT_TRUE(SStartsWith(response2.methodLine, "200 OK"));
        ASSERT_EQUAL(response2["message"], "Hello, World!");
    }

    void Core_MessagesCRUD() {
        const string corePluginPath = string(CORE_TEST_PLUGIN_DIR) + "/Core.so";
        map<string, string> args = {
            {"-plugins", "DB," + corePluginPath},
            {"-db", BedrockTester::getTempFileName("coretest")}
        };

        BedrockTester tester(args, {}, 0, 0, 0, true, CORE_TEST_BEDROCK_BIN);

        const string messageText = "Integration test " + SToStr(STimeNow());

        // Create a message
        SData createRequest("CreateMessage");
        createRequest["name"] = "Tester";
        createRequest["message"] = messageText;
        SData createResponse = tester.executeWaitMultipleData({createRequest}, 1).front();
        ASSERT_TRUE(SStartsWith(createResponse.methodLine, "200 OK"));
        ASSERT_EQUAL(createResponse["result"], "stored");
        ASSERT_EQUAL(createResponse["message"], messageText);
        const string messageID = createResponse["messageID"];
        ASSERT_FALSE(messageID.empty());

        // Fetch messages and ensure the inserted message is present
        SData listRequest("GetMessages");
        listRequest["limit"] = "10";
        SData listResponse = tester.executeWaitMultipleData({listRequest}, 1).front();
        ASSERT_TRUE(SStartsWith(listResponse.methodLine, "200 OK"));
        ASSERT_FALSE(listResponse["messages"].empty());

        list<string> rows = SParseJSONArray(listResponse["messages"]);
        bool found = false;
        for (const string& row : rows) {
            const STable entry = SParseJSONObject(row);
            if (entry.at("messageID") == messageID) {
                ASSERT_EQUAL(entry.at("name"), "Tester");
                ASSERT_EQUAL(entry.at("message"), messageText);
                found = true;
                break;
            }
        }

        ASSERT_TRUE(found);
    }
} __CoreTester;

