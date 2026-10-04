cmake_minimum_required(VERSION 3.20)
project(MyLimitOrderBookTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# GoogleTest 1.18.0 is the current release as of October 2026.
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.18.0
)

# Prevent GoogleTest from overriding the parent project's CRT settings on Windows.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB LOB_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/LOB/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Process_Orders/*.cpp"
)

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/tests/test_*.cpp"
)

add_executable(lob_tests
    ${LOB_SOURCES}
    ${TEST_SOURCES}
)

target_include_directories(lob_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/LOB"
    "${CMAKE_CURRENT_SOURCE_DIR}/Process_Orders"
)

target_link_libraries(lob_tests PRIVATE GTest::gtest_main)

enable_testing()

include(GoogleTest)
gtest_discover_tests(lob_tests)
