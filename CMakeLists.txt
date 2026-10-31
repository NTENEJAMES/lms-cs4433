cmake_minimum_required(VERSION 3.14)
project(LMS VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SQLite3 REQUIRED)

# Business logic goes in a library so tests can use it
add_library(lms_core src/Greeter.cpp src/Database.cpp)
target_include_directories(lms_core PUBLIC src)
target_link_libraries(lms_core PUBLIC SQLite::SQLite3)

add_executable(lms src/main.cpp)
target_link_libraries(lms PRIVATE lms_core)

include(FetchContent)
FetchContent_Declare(googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(lms_tests tests/test_greeter.cpp tests/test_database.cpp)
target_link_libraries(lms_tests PRIVATE lms_core GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(lms_tests)
