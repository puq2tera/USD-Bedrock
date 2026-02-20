# Core Plugin Tests

C++ integration tests for the Core Bedrock plugin commands.

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
- `TestHelpers.h`: shared tester setup and command-level helper utilities.
- `tests/HelloWorldTest.h`: `HelloWorld` command coverage.
- `tests/MessagesTest.h`: `CreateMessage` and `GetMessages` coverage.
- `tests/PollsTest.h`: `CreatePoll`, `GetPoll`, `SubmitVote`, `EditPoll`, `DeletePoll` coverage.
- `tests/UsersTest.h`: `CreateUser`, `GetUser`, `EditUser`, `DeleteUser` coverage, including cascade checks.
