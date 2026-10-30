cmake_minimum_required(VERSION 3.16)
project(vscp_fetchcontent_test LANGUAGES NONE)

include(FetchContent)
set(FETCHCONTENT_TRY_FIND_PACKAGE_MODE ALWAYS)

file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/packages")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/packages/nlohmann_jsonConfig.cmake"
    "set(nlohmann_json_FOUND TRUE)\n")
list(PREPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_BINARY_DIR}/packages")

set(_vscp_sources_only)
if(NOT CMAKE_VERSION VERSION_LESS "3.18")
    list(APPEND _vscp_sources_only SOURCE_SUBDIR "__vscp_sources_only__")
endif()

foreach(_dependency mongoose nlohmann_json spdlog mustache maddy)
    string(TOUPPER "${_dependency}" _upper)
    set(_source_var "VSCP_TEST_${_upper}_SOURCE_DIR")
    if(NOT IS_DIRECTORY "${${_source_var}}")
        message(FATAL_ERROR "Missing dependency source directory: ${_source_var}")
    endif()
    FetchContent_Declare(${_dependency}
        SOURCE_DIR "${${_source_var}}" ${_vscp_sources_only})
    FetchContent_Declare(vscp_${_dependency}
        SOURCE_DIR "${${_source_var}}" ${_vscp_sources_only})
endforeach()

add_subdirectory(check first)
add_subdirectory(check second)
