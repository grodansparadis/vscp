cmake_minimum_required(VERSION 3.10)

# Level II register read/write (CLASS2.PROTOCOL) against a simulated node.
# No hardware is needed.
project(test_register_level2 LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(GTest REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_executable(unittest_register_level2 unittest.cpp)

# MDF used by the CUserRegisters test
target_compile_definitions(unittest_register_level2 PRIVATE
    TEST_MDF_JSON_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../mdfparser/json")

target_link_libraries(unittest_register_level2 PRIVATE
    vscp_common
    Threads::Threads
    GTest::GTest
    GTest::Main
)
