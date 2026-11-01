cmake_minimum_required(VERSION 3.14)
project(LMS VERSION 0.1 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- SQLite: compiled from the source copy in third_party/sqlite ---
# (works on Windows, Linux and Mac with no package manager, and offline)
add_library(sqlite3_lib STATIC third_party/sqlite/sqlite3.c)
target_include_directories(sqlite3_lib PUBLIC third_party/sqlite)
find_package(Threads REQUIRED)
target_link_libraries(sqlite3_lib PUBLIC Threads::Threads ${CMAKE_DL_LIBS})

# --- Business logic goes in a library so tests can use it ---
add_library(lms_core
  src/Greeter.cpp
  src/Database.cpp
  src/LateDays.cpp
  src/TenPercentPerDayPolicy.cpp)
target_include_directories(lms_core PUBLIC src)
target_link_libraries(lms_core PUBLIC sqlite3_lib)

# --- The program itself ---
add_executable(lms src/main.cpp)
target_link_libraries(lms PRIVATE lms_core)

# --- GoogleTest (downloaded automatically) ---
include(FetchContent)
FetchContent_Declare(googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()
add_executable(lms_tests
  tests/test_greeter.cpp
  tests/test_database.cpp
  tests/test_penalty.cpp)
target_link_libraries(lms_tests PRIVATE lms_core GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(lms_tests)
