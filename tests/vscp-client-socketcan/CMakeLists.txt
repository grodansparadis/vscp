cmake_minimum_required(VERSION 3.10)

# set the project name
project(test_vscp_client_socketcan LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

message(STATUS "Build dir: " ${PROJECT_SOURCE_DIR})

find_package(GTest REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# add the executable
add_executable(unittest_vscp_client_socketcan unittest.cpp)

# vscp_common on Linux includes vscp-client-socketcan
target_link_libraries(unittest_vscp_client_socketcan PRIVATE
    vscp_common
    Threads::Threads
    GTest::GTest
    GTest::Main
)
