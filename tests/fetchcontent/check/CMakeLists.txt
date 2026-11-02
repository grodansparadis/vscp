include("${VSCP_SOURCE_ROOT}/cmake/VSCPDependencies.cmake")

FetchContent_GetProperties(vscp_mongoose POPULATED _vscp_mongoose_populated)
if(_vscp_mongoose_populated)
    message(FATAL_ERROR "VSCP must not fetch Mongoose")
endif()

if(NOT FETCHCONTENT_TRY_FIND_PACKAGE_MODE STREQUAL "ALWAYS")
    message(FATAL_ERROR "The parent project's FetchContent mode was changed")
endif()
if(NOT EXISTS "${VSCP_NLOHMANN_INCLUDE_DIR}/nlohmann/json.hpp"
   OR NOT EXISTS "${VSCP_SPDLOG_INCLUDE_DIR}/spdlog/spdlog.h"
   OR NOT EXISTS "${VSCP_MUSTACHE_INCLUDE_DIR}/mustache.hpp"
   OR NOT EXISTS "${VSCP_MADDY_INCLUDE_DIR}/maddy/parser.h")
    message(FATAL_ERROR "VSCP dependency paths are unavailable in ${CMAKE_CURRENT_SOURCE_DIR}")
endif()
