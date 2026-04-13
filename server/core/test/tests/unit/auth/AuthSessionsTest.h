#pragma once

#include "../../../TestHelpers.h"

struct AuthSessionsTest : tpunit::TestFixture {
    AuthSessionsTest()
        : tpunit::TestFixture(
            "AuthSessionsTest",
            TEST(AuthSessionsTest::testCreateSession),
            TEST(AuthSessionsTest::testCreateSessionRequiresExistingUser),
            TEST(AuthSessionsTest::testLookupByRefreshTokenHash),
            TEST(AuthSessionsTest::testLookupByRefreshTokenHashNotFound),
            TEST(AuthSessionsTest::testRotateSessionInvalidatesOldToken),
            TEST(AuthSessionsTest::testRevokeSessionRejectsSecondRevoke),
            TEST(AuthSessionsTest::testExpiredSessionBlocksRotationAndRevocation)
        ) { }

    string refreshTokenHash(const string& refreshToken) {
        return SToLower(SToHex(SHashSHA256(refreshToken)));
    }

    SData createSession(BedrockTester& tester,
                        const string& userID,
                        const string& refreshToken,
                        int64_t ttlSeconds = 3600,
                        const string& userAgent = "") {
        SData request("CreateUserSession");
        request["userID"] = userID;
        request["refreshTokenHash"] = refreshTokenHash(refreshToken);
        request["expiresAt"] = SToStr(STimeNow() + (STIME_US_PER_S * ttlSeconds));
        if (!userAgent.empty()) {
            request["userAgent"] = userAgent;
        }

        return TestHelpers::executeSingle(tester, request);
    }

    SData lookupSession(BedrockTester& tester, const string& refreshToken) {
        SData request("GetUserSessionByRefreshTokenHash");
        request["refreshTokenHash"] = refreshTokenHash(refreshToken);
        return TestHelpers::executeSingle(tester, request);
    }

    void testCreateSession() {
        BedrockTester tester = TestHelpers::createTester();
        const string userID = TestHelpers::createUserID(tester, "session-create", "Session", "Create");

        const SData response = createSession(tester, userID, "refresh-token-create", 3600, "UnitTest/CreateSession");

        ASSERT_TRUE(SStartsWith(response.methodLine, "200 OK"));
        ASSERT_FALSE(response["sessionID"].empty());
        ASSERT_EQUAL(response["userID"], userID);
        ASSERT_EQUAL(response["userAgent"], "UnitTest/CreateSession");
        ASSERT_TRUE(SToInt64(response["expiresAt"]) > SToInt64(response["createdAt"]));
    }

    void testCreateSessionRequiresExistingUser() {
        BedrockTester tester = TestHelpers::createTester();

        const SData response = createSession(tester, "99999", "refresh-token-missing-user");

        ASSERT_TRUE(SStartsWith(response.methodLine, "404"));
        ASSERT_EQUAL(response["errorCode"], "CREATE_USER_SESSION_USER_NOT_FOUND");
    }

    void testLookupByRefreshTokenHash() {
        BedrockTester tester = TestHelpers::createTester();
        const string userID = TestHelpers::createUserID(tester, "session-lookup", "Session", "Lookup");

        const SData created = createSession(tester, userID, "refresh-token-lookup", 3600, "UnitTest/Lookup");
        ASSERT_TRUE(SStartsWith(created.methodLine, "200 OK"));

        const SData lookup = lookupSession(tester, "refresh-token-lookup");

        ASSERT_TRUE(SStartsWith(lookup.methodLine, "200 OK"));
        ASSERT_EQUAL(lookup["sessionID"], created["sessionID"]);
        ASSERT_EQUAL(lookup["userID"], userID);
        ASSERT_EQUAL(lookup["refreshTokenHash"], refreshTokenHash("refresh-token-lookup"));
        ASSERT_EQUAL(lookup["userAgent"], "UnitTest/Lookup");
    }

    void testLookupByRefreshTokenHashNotFound() {
        BedrockTester tester = TestHelpers::createTester();

        const SData response = lookupSession(tester, "missing-refresh-token");

        ASSERT_TRUE(SStartsWith(response.methodLine, "404"));
        ASSERT_EQUAL(response["errorCode"], "GET_USER_SESSION_BY_HASH_NOT_FOUND");
    }

    void testRotateSessionInvalidatesOldToken() {
        BedrockTester tester = TestHelpers::createTester();
        const string userID = TestHelpers::createUserID(tester, "session-rotate", "Session", "Rotate");

        const SData created = createSession(tester, userID, "refresh-token-original", 3600, "UnitTest/Original");
        ASSERT_TRUE(SStartsWith(created.methodLine, "200 OK"));

        SData rotateRequest("RotateUserSession");
        rotateRequest["sessionID"] = created["sessionID"];
        rotateRequest["refreshTokenHash"] = refreshTokenHash("refresh-token-rotated");
        rotateRequest["expiresAt"] = SToStr(STimeNow() + (STIME_US_PER_S * 7200));
        const SData rotated = TestHelpers::executeSingle(tester, rotateRequest);

        ASSERT_TRUE(SStartsWith(rotated.methodLine, "200 OK"));
        ASSERT_FALSE(rotated["sessionID"].empty());
        ASSERT_TRUE(rotated["sessionID"] != created["sessionID"]);
        ASSERT_EQUAL(rotated["userID"], userID);
        ASSERT_EQUAL(rotated["replacedSessionID"], created["sessionID"]);
        ASSERT_EQUAL(rotated["userAgent"], "UnitTest/Original");

        const SData oldLookup = lookupSession(tester, "refresh-token-original");
        ASSERT_TRUE(SStartsWith(oldLookup.methodLine, "200 OK"));
        ASSERT_FALSE(oldLookup["revokedAt"].empty());
        ASSERT_EQUAL(oldLookup["replacedBySessionID"], rotated["sessionID"]);

        const SData newLookup = lookupSession(tester, "refresh-token-rotated");
        ASSERT_TRUE(SStartsWith(newLookup.methodLine, "200 OK"));
        ASSERT_EQUAL(newLookup["sessionID"], rotated["sessionID"]);
        ASSERT_EQUAL(newLookup["userID"], userID);
        ASSERT_EQUAL(newLookup["userAgent"], "UnitTest/Original");
    }

    void testRevokeSessionRejectsSecondRevoke() {
        BedrockTester tester = TestHelpers::createTester();
        const string userID = TestHelpers::createUserID(tester, "session-revoke", "Session", "Revoke");

        const SData created = createSession(tester, userID, "refresh-token-revoke");
        ASSERT_TRUE(SStartsWith(created.methodLine, "200 OK"));

        SData revokeRequest("RevokeUserSession");
        revokeRequest["sessionID"] = created["sessionID"];
        const SData firstRevoke = TestHelpers::executeSingle(tester, revokeRequest);

        ASSERT_TRUE(SStartsWith(firstRevoke.methodLine, "200 OK"));
        ASSERT_EQUAL(firstRevoke["result"], "revoked");
        ASSERT_EQUAL(firstRevoke["sessionID"], created["sessionID"]);

        const SData secondRevoke = TestHelpers::executeSingle(tester, revokeRequest);
        ASSERT_TRUE(SStartsWith(secondRevoke.methodLine, "409"));
        ASSERT_EQUAL(secondRevoke["errorCode"], "REVOKE_USER_SESSION_INVALID");
    }

    void testExpiredSessionBlocksRotationAndRevocation() {
        BedrockTester tester = TestHelpers::createTester();
        const string userID = TestHelpers::createUserID(tester, "session-expired", "Session", "Expired");

        const SData created = createSession(tester, userID, "refresh-token-expired");
        ASSERT_TRUE(SStartsWith(created.methodLine, "200 OK"));

        // Force the session into an expired state so both commands exercise the same invalid-session rule.
        TestHelpers::executeQuery(
            tester,
            "UPDATE user_sessions SET expiresAt = " + SToStr(STimeNow() - 1) + " WHERE sessionID = " + created["sessionID"]
        );

        SData rotateRequest("RotateUserSession");
        rotateRequest["sessionID"] = created["sessionID"];
        rotateRequest["refreshTokenHash"] = refreshTokenHash("refresh-token-expired-next");
        rotateRequest["expiresAt"] = SToStr(STimeNow() + (STIME_US_PER_S * 7200));
        const SData rotateResponse = TestHelpers::executeSingle(tester, rotateRequest);
        ASSERT_TRUE(SStartsWith(rotateResponse.methodLine, "409"));
        ASSERT_EQUAL(rotateResponse["errorCode"], "ROTATE_USER_SESSION_INVALID");

        SData revokeRequest("RevokeUserSession");
        revokeRequest["sessionID"] = created["sessionID"];
        const SData revokeResponse = TestHelpers::executeSingle(tester, revokeRequest);
        ASSERT_TRUE(SStartsWith(revokeResponse.methodLine, "409"));
        ASSERT_EQUAL(revokeResponse["errorCode"], "REVOKE_USER_SESSION_INVALID");
    }
};
