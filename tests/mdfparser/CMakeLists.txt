cmake_minimum_required(VERSION 3.10)

# set the project name
project(unittest_mdfparser LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

message(STATUS "Build dir: " ${PROJECT_SOURCE_DIR})

find_package(GTest REQUIRED)

# add the executable
add_executable(unittest_mdfparser unittest.cpp)

# vscp_common already links CURL and Expat (needed by mdf.cpp)
target_link_libraries(unittest_mdfparser PRIVATE
    vscp_common
    GTest::GTest
    GTest::Main
)
