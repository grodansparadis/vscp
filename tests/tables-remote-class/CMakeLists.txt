cmake_minimum_required(VERSION 3.16)
project(test_tables_remote_class LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Python3 COMPONENTS Interpreter REQUIRED)

if(NOT TARGET vscp_client_tcp)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../.. ${CMAKE_CURRENT_BINARY_DIR}/vscp)
endif()

add_executable(test_tables_remote_class test_tables_remote_class.cpp)
target_link_libraries(test_tables_remote_class PRIVATE vscp_client_tcp)

add_custom_target(check_tables_remote_class
    COMMAND ${Python3_EXECUTABLE} -B ${CMAKE_CURRENT_SOURCE_DIR}/../tcpiptls/run_table_tests.py
            $<TARGET_FILE:test_tables_remote_class>
    DEPENDS test_tables_remote_class
    COMMENT "Running remote table loopback checks"
    VERBATIM)
