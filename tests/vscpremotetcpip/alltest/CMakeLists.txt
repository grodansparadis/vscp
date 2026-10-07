cmake_minimum_required(VERSION 3.10)

# set the project name
project(unittest LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

message(STATUS "Build dir: " ${PROJECT_SOURCE_DIR})

find_package(GTest REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# Standalone build: pull in the VSCP library targets from the repository root
if(NOT TARGET vscp_core)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../.. ${CMAKE_CURRENT_BINARY_DIR}/vscp)
endif()

add_executable(unittest ${PROJECT_SOURCE_DIR}/unittest.cpp)

target_link_libraries(unittest PRIVATE
    vscp_core
    Threads::Threads
    GTest::GTest
    GTest::Main
)

