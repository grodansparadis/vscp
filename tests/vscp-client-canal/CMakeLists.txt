cmake_minimum_required(VERSION 3.10)

# set the project name
project(test_vscp_client_canal LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

message(STATUS "Build dir: " ${PROJECT_SOURCE_DIR})

find_package(GTest REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# add the executable
add_executable(unittest_vscp_client_canal unittest.cpp)

target_link_libraries(unittest_vscp_client_canal PRIVATE
    vscp_common
    Threads::Threads
    GTest::GTest
    GTest::Main
)

# Fake CANAL driver used by the filter tests
add_library(vscp-fakecanal MODULE fakecanal.cpp)
target_include_directories(vscp-fakecanal PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../src/vscp/common)
add_dependencies(unittest_vscp_client_canal vscp-fakecanal)
target_compile_definitions(unittest_vscp_client_canal PRIVATE
    FAKE_CANAL_PATH="$<TARGET_FILE:vscp-fakecanal>")
if(UNIX)
  target_link_libraries(unittest_vscp_client_canal PRIVATE dl)
endif()
