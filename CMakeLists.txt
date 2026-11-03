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

# --- Argon2 (password hashing), official reference code pinned to an exact commit ---
include(FetchContent)
FetchContent_Declare(argon2
  GIT_REPOSITORY https://github.com/P-H-C/phc-winner-argon2.git
  GIT_TAG 62358ba2123abd17fccf2a108a301d4b52c01a7c)
FetchContent_MakeAvailable(argon2)
add_library(argon2_lib STATIC
  ${argon2_SOURCE_DIR}/src/argon2.c
  ${argon2_SOURCE_DIR}/src/core.c
  ${argon2_SOURCE_DIR}/src/encoding.c
  ${argon2_SOURCE_DIR}/src/ref.c
  ${argon2_SOURCE_DIR}/src/blake2/blake2b.c)
target_include_directories(argon2_lib PUBLIC ${argon2_SOURCE_DIR}/include)
target_include_directories(argon2_lib PRIVATE ${argon2_SOURCE_DIR}/src)
target_compile_definitions(argon2_lib PRIVATE ARGON2_NO_THREADS)

# --- Business logic goes in a library so tests can use it ---
add_library(lms_core
  src/Greeter.cpp
  src/Database.cpp
  src/LateDays.cpp
  src/TenPercentPerDayPolicy.cpp
  src/User.cpp
  src/Roles.cpp
  src/Assignment.cpp
  src/Submission.cpp
  src/TimeUtil.cpp
  src/Statement.cpp
  src/UserFactory.cpp
  src/SqliteUserRepository.cpp
  src/SqliteSubmissionRepository.cpp
  src/PasswordHasher.cpp
  src/AuthService.cpp
  src/SqliteAssignmentRepository.cpp
  src/SqliteCourseRepository.cpp
  src/SubmissionService.cpp
  src/SqliteAuditRepository.cpp
  src/MarkingService.cpp)
target_include_directories(lms_core PUBLIC src)
target_link_libraries(lms_core PUBLIC sqlite3_lib argon2_lib)
if(WIN32)
  target_link_libraries(lms_core PUBLIC bcrypt)   # secure random numbers on Windows
endif()

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
  tests/test_penalty.cpp
  tests/test_schema.cpp
  tests/test_users.cpp
  tests/test_assignment.cpp
  tests/test_submission.cpp
  tests/test_timeutil.cpp
  tests/test_repositories.cpp
  tests/test_auth.cpp
  tests/test_assignment_repository.cpp
  tests/test_submission_service.cpp
  tests/test_marking_service.cpp)
target_link_libraries(lms_tests PRIVATE lms_core GTest::gtest_main)
target_compile_definitions(lms_tests PRIVATE LMS_SCHEMA_PATH="${CMAKE_SOURCE_DIR}/db/schema.sql")
include(GoogleTest)
gtest_discover_tests(lms_tests)
