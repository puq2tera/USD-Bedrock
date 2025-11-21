#pragma once

#include <libstuff/libstuff.h>
#include <test/lib/BedrockTester.h>
#include <test/lib/tpunit++.hpp>

#include <map>
#include <vector>

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
            TEST(CoreTester::Core_HelloWorld)
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
        ASSERT_EQUAL(response["plugin_version"], "1.0.0");

        SData request2("HelloWorld");
        vector<SData> responses2 = tester.executeWaitMultipleData({request2}, 1);
        SData response2 = responses2.front();
        ASSERT_TRUE(SStartsWith(response2.methodLine, "200 OK"));
        ASSERT_EQUAL(response2["message"], "Hello, World!");
    }
} __CoreTester;

