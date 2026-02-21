#pragma once

#include "../../../TestHelpers.h"

struct ChatMembersTest : tpunit::TestFixture {
    ChatMembersTest()
        : tpunit::TestFixture(
            "ChatMembersTests",
            TEST(ChatMembersTest::testAddChatMemberSuccessWithDefaultRole),
            TEST(ChatMembersTest::testAddChatMemberForbiddenForNonOwner),
            TEST(ChatMembersTest::testAddChatMemberRejectsInvalidRole),
            TEST(ChatMembersTest::testAddChatMemberRejectsDuplicate),
            TEST(ChatMembersTest::testListChatMembersOwnerOnly),
            TEST(ChatMembersTest::testEditChatMemberRoleAndLastOwnerProtection),
            TEST(ChatMembersTest::testEditChatMemberRoleRejectsInvalidRole),
            TEST(ChatMembersTest::testEditChatMemberRoleTargetMustExist),
            TEST(ChatMembersTest::testRemoveChatMemberLastOwnerProtection),
            TEST(ChatMembersTest::testRemoveChatMemberTargetMustExist)
        ) { }

    void testAddChatMemberSuccessWithDefaultRole() {
        BedrockTester tester = TestHelpers::createTester();
        const string ownerID = TestHelpers::createUserID(tester, "owner", "Owner", "User");
        const string candidateID = TestHelpers::createUserID(tester, "candidate", "Candidate", "User");
        const string chatID = TestHelpers::createChatID(tester, ownerID, "Team chat");

        SData addReq("AddChatMember");
        addReq["chatID"] = chatID;
        addReq["actingUserID"] = ownerID;
        addReq["userID"] = candidateID;
        SData addResp = TestHelpers::executeSingle(tester, addReq);

        ASSERT_TRUE(SStartsWith(addResp.methodLine, "200 OK"));
        ASSERT_EQUAL(addResp["result"], "added");
        ASSERT_EQUAL(addResp["role"], "member");

        SData listReq("ListChatMembers");
        listReq["chatID"] = chatID;
        listReq["userID"] = ownerID;
        SData listResp = TestHelpers::executeSingle(tester, listReq);
        ASSERT_TRUE(SStartsWith(listResp.methodLine, "200 OK"));

        list<string> members = SParseJSONArray(listResp["members"]);
        bool found = false;
        for (const string& row : members) {
            const STable member = SParseJSONObject(row);
            if (member.at("userID") == candidateID) {
                ASSERT_EQUAL(member.at("role"), "member");
                found = true;
                break;
            }
        }
        ASSERT_TRUE(found);
    }

    void testAddChatMemberForbiddenForNonOwner() {
        BedrockTester tester = TestHelpers::createTester();
        const string ownerID = TestHelpers::createUserID(tester, "owner", "Owner", "User");
        const string memberID = TestHelpers::createUserID(tester, "member", "Member", "User");
        const string candidateID = TestHelpers::createUserID(tester, "candidate", "Candidate", "User");
        const string chatID = TestHelpers::createChatID(tester, ownerID, "Team chat");

        TestHelpers::addChatMember(tester, chatID, ownerID, memberID, "member");

        SData addReq("AddChatMember");
        addReq["chatID"] = chatID;
        addReq["actingUserID"] = memberID;
        addReq["userID"] = candidateID;
        SData addResp = TestHelpers::executeSingle(tester, addReq);

        ASSERT_TRUE(SStartsWith(addResp.methodLine, "403"));
    }

    void testAddChatMemberRejectsInvalidRole() {
        BedrockTester tester = TestHelpers::createTester();
        const string ownerID = TestHelpers::createUserID(tester, "owner", "Owner", "User");
        const string candidateID = TestHelpers::createUserID(tester, "candidate", "Candidate", "User");
        const string chatID = TestHelpers::createChatID(tester, ownerID, "Team chat");

        SData addReq("AddChatMember");
        addReq["chatID"] = chatID;
        addReq["actingUserID"] = ownerID;
        addReq["userID"] = candidateID;
        addReq["role"] = "admin";
        SData addResp = TestHelpers::executeSingle(tester, addReq);

        ASSERT_TRUE(SStartsWith(addResp.methodLine, "400"));
    }

    void testAddChatMemberRejectsDuplicate() {
        BedrockTester tester = TestHelpers::createTester();
        const string ownerID = TestHelpers::createUserID(tester, "owner", "Owner", "User");
        const string candidateID = TestHelpers::createUserID(tester, "candidate", "Candidate", "User");
        const string chatID = TestHelpers::createChatID(tester, ownerID, "Team chat");

        SData firstAddReq("AddChatMember");
        firstAddReq["chatID"] = chatID;
        firstAddReq["actingUserID"] = ownerID;
        firstAddReq["userID"] = candidateID;
        SData firstAddResp = TestHelpers::executeSingle(tester, firstAddReq);
        ASSERT_TRUE(SStartsWith(firstAddResp.methodLine, "200 OK"));

        SData duplicateReq("AddChatMember");
        duplicateReq["chatID"] = chatID;
        duplicateReq["actingUserID"] = ownerID;
        duplicateReq["userID"] = candidateID;
        SData duplicateResp = TestHelpers::executeSingle(tester, duplicateReq);

        ASSERT_TRUE(SStartsWith(duplicateResp.methodLine, "409"));
    }

    void testListChatMembersOwnerOnly() {
        BedrockTester tester = TestHelpers::createTester();
        const string ownerID = TestHelpers::createUserID(tester, "owner", "Owner", "User");
        const string memberID = TestHelpers::createUserID(tester, "member", "Member", "User");
        const string chatID = TestHelpers::createChatID(tester, ownerID, "Members");

        TestHelpers::addChatMember(tester, chatID, ownerID, memberID, "member");

        SData listAsMemberReq("ListChatMembers");
        listAsMemberReq["chatID"] = chatID;
        listAsMemberReq["userID"] = memberID;
        SData listAsMemberResp = TestHelpers::executeSingle(tester, listAsMemberReq);

        ASSERT_TRUE(SStartsWith(listAsMemberResp.methodLine, "403"));

        SData listAsOwnerReq("ListChatMembers");
        listAsOwnerReq["chatID"] = chatID;
        listAsOwnerReq["userID"] = ownerID;
        SData listAsOwnerResp = TestHelpers::executeSingle(tester, listAsOwnerReq);

        ASSERT_TRUE(SStartsWith(listAsOwnerResp.methodLine, "200 OK"));
        list<string> members = SParseJSONArray(listAsOwnerResp["members"]);
        ASSERT_EQUAL(members.size(), static_cast<size_t>(2));
    }

    void testEditChatMemberRoleAndLastOwnerProtection() {
        BedrockTester tester = TestHelpers::createTester();
        const string owner1ID = TestHelpers::createUserID(tester, "owner1", "Owner", "One");
        const string owner2ID = TestHelpers::createUserID(tester, "owner2", "Owner", "Two");
        const string chatID = TestHelpers::createChatID(tester, owner1ID, "Owners");

        TestHelpers::addChatMember(tester, chatID, owner1ID, owner2ID, "owner");

        SData demoteOwner1Req("EditChatMemberRole");
        demoteOwner1Req["chatID"] = chatID;
        demoteOwner1Req["actingUserID"] = owner2ID;
        demoteOwner1Req["userID"] = owner1ID;
        demoteOwner1Req["role"] = "member";
        SData demoteOwner1Resp = TestHelpers::executeSingle(tester, demoteOwner1Req);

        ASSERT_TRUE(SStartsWith(demoteOwner1Resp.methodLine, "200 OK"));
        ASSERT_EQUAL(demoteOwner1Resp["role"], "member");

        SData demoteLastOwnerReq("EditChatMemberRole");
        demoteLastOwnerReq["chatID"] = chatID;
        demoteLastOwnerReq["actingUserID"] = owner2ID;
        demoteLastOwnerReq["userID"] = owner2ID;
        demoteLastOwnerReq["role"] = "member";
        SData demoteLastOwnerResp = TestHelpers::executeSingle(tester, demoteLastOwnerReq);

        ASSERT_TRUE(SStartsWith(demoteLastOwnerResp.methodLine, "409"));
    }

    void testEditChatMemberRoleRejectsInvalidRole() {
        BedrockTester tester = TestHelpers::createTester();
        const string ownerID = TestHelpers::createUserID(tester, "owner", "Owner", "One");
        const string memberID = TestHelpers::createUserID(tester, "member", "Member", "One");
        const string chatID = TestHelpers::createChatID(tester, ownerID, "Members");
        TestHelpers::addChatMember(tester, chatID, ownerID, memberID, "member");

        SData editReq("EditChatMemberRole");
        editReq["chatID"] = chatID;
        editReq["actingUserID"] = ownerID;
        editReq["userID"] = memberID;
        editReq["role"] = "admin";
        const SData editResp = TestHelpers::executeSingle(tester, editReq);

        ASSERT_TRUE(SStartsWith(editResp.methodLine, "400"));
    }

    void testEditChatMemberRoleTargetMustExist() {
        BedrockTester tester = TestHelpers::createTester();
        const string ownerID = TestHelpers::createUserID(tester, "owner", "Owner", "One");
        const string chatID = TestHelpers::createChatID(tester, ownerID, "Owners");

        SData editReq("EditChatMemberRole");
        editReq["chatID"] = chatID;
        editReq["actingUserID"] = ownerID;
        editReq["userID"] = "99999";
        editReq["role"] = "member";
        SData editResp = TestHelpers::executeSingle(tester, editReq);

        ASSERT_TRUE(SStartsWith(editResp.methodLine, "403"));
    }

    void testRemoveChatMemberLastOwnerProtection() {
        BedrockTester tester = TestHelpers::createTester();
        const string owner1ID = TestHelpers::createUserID(tester, "owner1", "Owner", "One");
        const string owner2ID = TestHelpers::createUserID(tester, "owner2", "Owner", "Two");
        const string chatID = TestHelpers::createChatID(tester, owner1ID, "Owners");

        SData removeSelfReq("RemoveChatMember");
        removeSelfReq["chatID"] = chatID;
        removeSelfReq["actingUserID"] = owner1ID;
        removeSelfReq["userID"] = owner1ID;
        SData removeSelfResp = TestHelpers::executeSingle(tester, removeSelfReq);

        ASSERT_TRUE(SStartsWith(removeSelfResp.methodLine, "409"));

        TestHelpers::addChatMember(tester, chatID, owner1ID, owner2ID, "owner");

        SData removeOwner1Req("RemoveChatMember");
        removeOwner1Req["chatID"] = chatID;
        removeOwner1Req["actingUserID"] = owner2ID;
        removeOwner1Req["userID"] = owner1ID;
        SData removeOwner1Resp = TestHelpers::executeSingle(tester, removeOwner1Req);

        ASSERT_TRUE(SStartsWith(removeOwner1Resp.methodLine, "200 OK"));
        ASSERT_EQUAL(removeOwner1Resp["result"], "removed");
    }

    void testRemoveChatMemberTargetMustExist() {
        BedrockTester tester = TestHelpers::createTester();
        const string ownerID = TestHelpers::createUserID(tester, "owner", "Owner", "One");
        const string chatID = TestHelpers::createChatID(tester, ownerID, "Owners");

        SData removeReq("RemoveChatMember");
        removeReq["chatID"] = chatID;
        removeReq["actingUserID"] = ownerID;
        removeReq["userID"] = "99999";
        SData removeResp = TestHelpers::executeSingle(tester, removeReq);

        ASSERT_TRUE(SStartsWith(removeResp.methodLine, "403"));
    }
};
