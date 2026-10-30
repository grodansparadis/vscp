include_guard()
include(FetchContent)

set(_VSCP_FETCH_OPTIONS)
if(NOT CMAKE_VERSION VERSION_LESS "3.18")
    # Populate the sources without adding their CMake projects.
    list(APPEND _VSCP_FETCH_OPTIONS SOURCE_SUBDIR "__vscp_sources_only__")
endif()
if(NOT CMAKE_VERSION VERSION_LESS "3.24")
    list(APPEND _VSCP_FETCH_OPTIONS DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
endif()

# FetchContent names are global to the build, so keep these separate from
# dependencies declared by a parent project.
FetchContent_Declare(
    vscp_mongoose
    URL https://codeload.github.com/cesanta/mongoose/tar.gz/c13d5d7a569aade248983e6abb1c0b41b1899f9b
    URL_HASH SHA256=4c28986a5dea2152e0959a3913b9327990658f0caf3826e284ffe13a9f058439
    ${_VSCP_FETCH_OPTIONS}
)
FetchContent_Declare(
    vscp_nlohmann_json
    URL https://codeload.github.com/nlohmann/json/tar.gz/bfb07786cd7eb841a6d9030bcd822d3b1c4d3b56
    URL_HASH SHA256=46e944d2d2524ae702df4d44ee963bd6640ed9ee1319ac5a72030336c11c91de
    ${_VSCP_FETCH_OPTIONS}
)
FetchContent_Declare(
    vscp_spdlog
    URL https://codeload.github.com/gabime/spdlog/tar.gz/57cb5fb7a8ff30079751728234623230535a5c92
    URL_HASH SHA256=e7a5ba64c021d0889e31d9a03b9296b5db1663c28293cf8a7ea173abd66bcd0c
    ${_VSCP_FETCH_OPTIONS}
)
FetchContent_Declare(
    vscp_mustache
    URL https://codeload.github.com/kainjow/Mustache/tar.gz/3f654942a70c46a775070d7a09ca7acfa3e205b7
    URL_HASH SHA256=959b8fcaecc14fc0b6243c078a39fd13dab107dcda086ed2ca90e1a486269a36
    ${_VSCP_FETCH_OPTIONS}
)
FetchContent_Declare(
    vscp_maddy
    URL https://codeload.github.com/progsource/maddy/tar.gz/4dcdbb50be10a6262072340abcfa97ee3b4edcfa
    URL_HASH SHA256=683633e542b8e5aa56823875d7975cdc7993729c0ec88163c487ba69256ce18c
    ${_VSCP_FETCH_OPTIONS}
)

set(_VSCP_DEPENDENCIES vscp_mongoose vscp_nlohmann_json vscp_spdlog vscp_mustache vscp_maddy)
if(CMAKE_VERSION VERSION_LESS "3.18")
    foreach(_vscp_dependency IN LISTS _VSCP_DEPENDENCIES)
        FetchContent_GetProperties(${_vscp_dependency})
        if(NOT ${_vscp_dependency}_POPULATED)
            FetchContent_Populate(${_vscp_dependency})
        endif()
    endforeach()
else()
    FetchContent_MakeAvailable(${_VSCP_DEPENDENCIES})
endif()

foreach(_vscp_dependency IN LISTS _VSCP_DEPENDENCIES)
    if(NOT ${_vscp_dependency}_SOURCE_DIR)
        string(TOUPPER "${_vscp_dependency}" _vscp_dependency_upper)
        message(FATAL_ERROR
            "VSCP requires the ${_vscp_dependency} source tree. Set "
            "FETCHCONTENT_SOURCE_DIR_${_vscp_dependency_upper} to an existing checkout.")
    endif()
endforeach()

set(VSCP_MONGOOSE_SOURCE_DIR "${vscp_mongoose_SOURCE_DIR}")
set(VSCP_NLOHMANN_INCLUDE_DIR "${vscp_nlohmann_json_SOURCE_DIR}/include")
set(VSCP_SPDLOG_INCLUDE_DIR "${vscp_spdlog_SOURCE_DIR}/include")
set(VSCP_MUSTACHE_INCLUDE_DIR "${vscp_mustache_SOURCE_DIR}")
set(VSCP_MADDY_INCLUDE_DIR "${vscp_maddy_SOURCE_DIR}/include")
set(VSCP_HAVE_MADDY ON)

foreach(_vscp_dependency_file
        "${VSCP_MONGOOSE_SOURCE_DIR}/mongoose.c"
        "${VSCP_MONGOOSE_SOURCE_DIR}/mongoose.h"
        "${VSCP_NLOHMANN_INCLUDE_DIR}/nlohmann/json.hpp"
        "${VSCP_SPDLOG_INCLUDE_DIR}/spdlog/spdlog.h"
        "${VSCP_MUSTACHE_INCLUDE_DIR}/mustache.hpp"
        "${VSCP_MADDY_INCLUDE_DIR}/maddy/parser.h")
    if(NOT EXISTS "${_vscp_dependency_file}")
        message(FATAL_ERROR "VSCP dependency file is missing: ${_vscp_dependency_file}")
    endif()
endforeach()
