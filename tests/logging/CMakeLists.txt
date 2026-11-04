find_package(GTest REQUIRED)

add_executable(unittest_core_logging unittest.cpp)
target_compile_definitions(unittest_core_logging PRIVATE VSCP_ENABLE_HELPER_LOGGING)
target_link_libraries(unittest_core_logging PRIVATE vscp_core GTest::GTest GTest::Main)

add_executable(logging_core_c_host logging_c_host.c)
target_link_libraries(logging_core_c_host PRIVATE vscp_core)

add_custom_target(test_core_logging
    COMMAND $<TARGET_FILE:unittest_core_logging>
    COMMAND $<TARGET_FILE:logging_core_c_host>
    DEPENDS unittest_core_logging logging_core_c_host
    USES_TERMINAL
    VERBATIM)

set(VSCP_TEST_HOST_SPDLOG_INCLUDE_DIR "" CACHE PATH
    "Optional alternate spdlog include directory for host compatibility testing")
if(VSCP_TEST_HOST_SPDLOG_INCLUDE_DIR)
    if(NOT EXISTS "${VSCP_TEST_HOST_SPDLOG_INCLUDE_DIR}/spdlog/version.h")
        message(FATAL_ERROR "VSCP_TEST_HOST_SPDLOG_INCLUDE_DIR must contain spdlog headers")
    endif()
    add_executable(logging_core_host_compat host_compat.cpp)
    target_include_directories(logging_core_host_compat BEFORE PRIVATE
        "${VSCP_TEST_HOST_SPDLOG_INCLUDE_DIR}")
    target_link_libraries(logging_core_host_compat PRIVATE vscp_core)
    add_custom_target(test_core_logging_host_compat
        COMMAND $<TARGET_FILE:logging_core_host_compat>
        DEPENDS logging_core_host_compat
        USES_TERMINAL
        VERBATIM)
endif()
