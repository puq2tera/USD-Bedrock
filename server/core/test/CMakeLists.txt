# Core plugin test suite
cmake_minimum_required(VERSION 3.10)

# Find all test source files
file(GLOB_RECURSE CORETEST_SRC "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# Find Bedrock test library files
file(GLOB TESTCPP
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../Bedrock/test/lib/*.cpp"
)

# Remove files we don't need
list(REMOVE_ITEM TESTCPP "${CMAKE_CURRENT_SOURCE_DIR}/../../../Bedrock/test/lib/TestHTTPS.cpp")
list(REMOVE_ITEM TESTCPP "${CMAKE_CURRENT_SOURCE_DIR}/../../../Bedrock/test/lib/TestPlugin.cpp")

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -fPIC")

# Skip linting for tpunit++
set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../Bedrock/test/lib/tpunit++.cpp
    PROPERTIES SKIP_LINTING ON
)

# Build the test executable
add_executable(coretest ${CORETEST_SRC} ${TESTCPP})
target_compile_options(coretest PRIVATE
    -Wall
    -Wextra
    -fPIC
    -Wno-gnu-zero-variadic-macro-arguments
    -Wno-missing-field-initializers
    -Wno-gnu-conditional-omitted-operand
    -Wno-unqualified-std-cast-call
    -Wno-ignored-qualifiers
    -Wno-unused-parameter
)
target_include_directories(coretest PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../Bedrock
    ${CMAKE_CURRENT_SOURCE_DIR}/../../
)
get_filename_component(CORE_PLUGIN_DIR "${CMAKE_BINARY_DIR}/lib" ABSOLUTE)
set(CORE_BEDROCK_BIN "${BEDROCK_DIR}/bedrock")
target_compile_definitions(coretest PRIVATE
    CORE_TEST_PLUGIN_DIR="${CORE_PLUGIN_DIR}"
    CORE_TEST_BEDROCK_BIN="${CORE_BEDROCK_BIN}"
)
target_link_libraries(coretest
    Core
    ${BEDROCK_DIR}/libbedrock.a
    ${BEDROCK_DIR}/libstuff.a
    ${BEDROCK_DIR}/mbedtls/library/libmbedtls.a
    ${BEDROCK_DIR}/mbedtls/library/libmbedx509.a
    ${BEDROCK_DIR}/mbedtls/library/libmbedcrypto.a
    pthread
    dl
    pcre2-8
    z
)
set_target_properties(coretest PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

