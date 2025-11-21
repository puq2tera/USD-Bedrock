# Core Plugin Tests

Basic test suite for the Bedrock Core plugin.

## Building and Running

Use the helper script:

```bash
./scripts/test-cpp.sh
```

Pass arguments (for example, to run a specific test):

```bash
./scripts/test-cpp.sh -only Core_HelloWorld
```

Enable verbose logging:

```bash
./scripts/test-cpp.sh -v
```

## Test Structure

- `main.cpp` - Test runner entry point
- `CoreTester.h` - Test fixtures for Core plugin commands
- `CMakeLists.txt` - Build configuration

## Adding Tests

Add new test methods to `CoreTester.h` following the pattern:

```cpp
void Core_MyNewTest() {
    BedrockTester tester = BedrockTester({{"-plugins", "Core"}}, {});
    
    SData request("MyCommand");
    request["param"] = "value";
    STable response = tester.executeWaitVerifyContentTable(request);
    
    ASSERT_EQUAL(response["expected"], "value");
}
```

Then add the test to the constructor:

```cpp
CoreTester() : tpunit::TestFixture(
    "Core_HelloWorld, Core_MyNewTest",
    AFTER(Core_HelloWorld, Core_MyNewTest)
) { }
```

