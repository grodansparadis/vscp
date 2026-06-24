cmake_minimum_required(VERSION 3.10)

# set the project name
project(test_guid LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

message(STATUS "Build dir: " ${PROJECT_SOURCE_DIR})

find_package(GTest REQUIRED)

# add the executable
add_executable(unittest_guid unittest.cpp)

target_link_libraries(unittest_guid PRIVATE
    vscp_common
    GTest::GTest
    GTest::Main
)
