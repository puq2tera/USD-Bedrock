# Core Plugin Tests

C++ tests for the Core Bedrock plugin commands.

## Run

```bash
./scripts/test-cpp.sh
```

Run a subset:

```bash
./scripts/test-cpp.sh -only testCreatePollSuccess
./scripts/test-cpp.sh -only testCreateUserSuccess,testEditUserPartial
./scripts/test-cpp.sh -except testGetMessagesDescendingOrder
```

Verbose logs:

```bash
./scripts/test-cpp.sh -v
```

## Layout

- `main.cpp`: test runner and fixture registration.
- `TestHelpers.h`: shared utilities only for broadly reusable test behavior. Keep command-specific helpers local to the relevant test file.
- `tests/unit/`: unit-style command tests, grouped by domain/table family.
- `tests/unit/system/HelloWorldTest.h`: `HelloWorld` command coverage.
- `tests/unit/users/UsersTest.h`: `CreateUser`, `GetUser`, `EditUser`, `DeleteUser` coverage, including cascade checks.
- `tests/unit/chats/ChatsTest.h`: `CreateChat`, `GetChat`, `ListChats`, `EditChat`, `DeleteChat`.
- `tests/unit/chats/ChatMembersTest.h`: `AddChatMember`, `ListChatMembers`, `EditChatMemberRole`, `RemoveChatMember`.
- `tests/unit/chats/ChatMessagesTest.h`: `CreateChatMessage`, `GetChatMessages`, `EditChatMessage`, `DeleteChatMessage`.
- `tests/unit/polls/PollsTest.h`: `CreatePoll`, `GetPoll`, `ListPolls`, `EditPoll`, `DeletePoll`.
- `tests/unit/polls/PollVotesTest.h`: `SubmitPollVotes`, `DeletePollVotes`.
- `tests/unit/polls/PollTextResponsesTest.h`: `SubmitPollTextResponse`.
- `tests/integration/`: integration-test area for future cross-feature test flows, with domain subfolders matching `tests/unit/`.
