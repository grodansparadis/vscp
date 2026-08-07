# CMakeLists.txt
#
# VSCP common library — build instructions.
#
# Defines the vscp_common and vscp_util static library targets
# for use as a subproject by the VSCP daemon and other consumers.
#
# Copyright (C) 2000-2026 Ake Hedman, the VSCP project
# SPDX-License-Identifier: MIT
#
# Usage as a subproject (e.g. from the vscp-daemon repo):
#   add_subdirectory(external/vscp)
#   target_link_libraries(myapp PRIVATE vscp_common)
#
# Standalone library build:
#   cmake -B build
#   cmake --build build
#
# Run tests:
#   cmake -B build -DVSCP_BUILD_TESTS=ON
#   cmake --build build
#
# Windows (vcpkg):
#   cmake .. -G "Visual Studio 17 2022" -DVCPKG_TARGET_TRIPLET=x64-windows \
#            -DCMAKE_TOOLCHAIN_FILE=<vcpkg_root>/scripts/buildsystems/vcpkg.cmake
#   cmake --build . --config Release
#

cmake_minimum_required(VERSION 3.16...3.31)
project(vscp
    VERSION 15.1.1
    DESCRIPTION "VSCP common library"
    LANGUAGES CXX C)

set(VSCP_VERSION_MAJOR 15)
set(VSCP_VERSION_MINOR 1)
set(VSCP_VERSION_PATCH 1)

# ---- Build options ----
option(VSCP_BUILD_TESTS "Build unit tests (use tests/ directory for standalone test builds)" OFF)
option(VSCP_INSTALL "Install VSCP library targets and headers" ${PROJECT_IS_TOP_LEVEL})

# ---- C++ standard ----
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Generate position-independent code (-fPIC on UNIX)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Build output locations
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

# ---- Module path ----
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules)

# ---- Third-party header roots ----
set(VSCP_NLOHMANN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party/nlohmann/include)
if(NOT EXISTS ${VSCP_NLOHMANN_INCLUDE_DIR}/nlohmann/json.hpp)
    set(VSCP_NLOHMANN_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../json/include)
endif()

set(VSCP_SPDLOG_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party/spdlog/include)
if(NOT EXISTS ${VSCP_SPDLOG_INCLUDE_DIR}/spdlog/spdlog.h)
    set(VSCP_SPDLOG_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../spdlog/include)
endif()

set(VSCP_MUSTACHE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party/mustache)
if(NOT EXISTS ${VSCP_MUSTACHE_INCLUDE_DIR}/mustache.hpp)
    set(VSCP_MUSTACHE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../Mustache)
endif()

set(VSCP_MADDY_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third-party/maddy/include)
set(VSCP_HAVE_MADDY OFF)
if(EXISTS ${VSCP_MADDY_INCLUDE_DIR}/maddy/parser.h)
    set(VSCP_HAVE_MADDY ON)
endif()

# ---- System libraries ----
include(GNUInstallDirs)

# ---- Find dependencies ----

# Threads
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# OpenSSL
find_package(OpenSSL REQUIRED)

# Determine OpenSSL API version compile definition
set(VSCP_OPENSSL_API_DEF "OPENSSL_API_1_1")
string(FIND "${OPENSSL_VERSION}" "1.0" _ssl_pos)
if(NOT _ssl_pos EQUAL -1)
    set(VSCP_OPENSSL_API_DEF "OPENSSL_API_1_0")
endif()
string(FIND "${OPENSSL_VERSION}" "1.1" _ssl_pos)
if(NOT _ssl_pos EQUAL -1)
    set(VSCP_OPENSSL_API_DEF "OPENSSL_API_1_1")
endif()
string(FIND "${OPENSSL_VERSION}" "3.0" _ssl_pos)
if(NOT _ssl_pos EQUAL -1)
    set(VSCP_OPENSSL_API_DEF "OPENSSL_API_3_0")
endif()
string(FIND "${OPENSSL_VERSION}" "3.1" _ssl_pos)
if(NOT _ssl_pos EQUAL -1)
    set(VSCP_OPENSSL_API_DEF "OPENSSL_API_3_1")
endif()
message(STATUS "VSCP: OpenSSL ${OPENSSL_VERSION} -> -D${VSCP_OPENSSL_API_DEF}")

# Expat
find_package(EXPAT REQUIRED)

# Mosquitto
if(WIN32)
    find_package(MOSQUITTO REQUIRED)
else()
    find_package(Mosquitto REQUIRED)
endif()

# CURL (needed by mdf.cpp)
find_package(CURL REQUIRED)

# ---- Platform-specific setup ----
if(WIN32)
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
    add_definitions(-D_WINSOCK_DEPRECATED_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS
                    -DWIN32 -D_WIN32 -DWIN32_LEAN_AND_MEAN -D_POSIX_C_SOURCE)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

    set(VSCP_PTHREAD_TARGET "")
    set(VSCP_WINDOWS_GETOPT_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/src/common/windows/getopt.c)
    set(VSCP_WINDOWS_GETOPT_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/common/windows)
    set(VSCP_WINDOWS_PCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/vscp/daemon)
    set(VSCP_WINDOWS_HELPERS_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/vscp/common/win32helpers)

    # vcpkg path handling
    if(DEFINED VCPKG_TARGET_TRIPLET)
        if(EXISTS "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}")
            list(PREPEND CMAKE_PREFIX_PATH "${CMAKE_BINARY_DIR}/vcpkg_installed/${VCPKG_TARGET_TRIPLET}")
        endif()
        if(DEFINED ENV{VCPKG_ROOT})
            list(PREPEND CMAKE_PREFIX_PATH "$ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}")
        endif()
        if(DEFINED VCPKG_INSTALLED_DIR)
            list(PREPEND CMAKE_PREFIX_PATH "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}")
        endif()
    endif()

    find_package(pthread REQUIRED)
    find_package(dlfcn-win32 CONFIG REQUIRED)
    if(TARGET PThreads4W::PThreads4W_CXXEXC)
        set(VSCP_PTHREAD_TARGET PThreads4W::PThreads4W_CXXEXC)
    endif()
elseif(APPLE)
    add_compile_options(-Wno-psabi)
    # mosquitto on macOS may depend on cJSON
    find_path(CJSON_INCLUDE_DIR cjson/cJSON.h
        HINTS /opt/homebrew/include /usr/local/include
              /opt/homebrew/opt/cjson/include /usr/local/opt/cjson/include)
    if(CJSON_INCLUDE_DIR)
        message(STATUS "VSCP: cJSON include dir: ${CJSON_INCLUDE_DIR}")
    endif()
else()
    # Linux
    add_compile_options(-Wno-psabi)
endif()

# =====================================================================
# Shared target setup for VSCP protocol/component libraries
# =====================================================================
function(vscp_setup_protocol_target target)
    target_include_directories(${target}
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/vscp/common>
            $<BUILD_INTERFACE:${VSCP_NLOHMANN_INCLUDE_DIR}>
            $<BUILD_INTERFACE:${VSCP_SPDLOG_INCLUDE_DIR}>
            $<BUILD_INTERFACE:${VSCP_MUSTACHE_INCLUDE_DIR}>
            $<BUILD_INTERFACE:${EXPAT_INCLUDE_DIRS}>
            $<BUILD_INTERFACE:${MOSQUITTO_INCLUDE_DIRS}>
            $<BUILD_INTERFACE:${CURL_INCLUDE_DIRS}>
            $<INSTALL_INTERFACE:include/vscp>
    )

    if(VSCP_HAVE_MADDY)
        target_include_directories(${target} PUBLIC
            $<BUILD_INTERFACE:${VSCP_MADDY_INCLUDE_DIR}>)
        target_compile_definitions(${target} PUBLIC VSCP_HAVE_MADDY)
    endif()

    if(APPLE AND CJSON_INCLUDE_DIR)
        target_include_directories(${target} PUBLIC
            $<BUILD_INTERFACE:${CJSON_INCLUDE_DIR}>)
    endif()

    if(WIN32)
        target_include_directories(${target} PUBLIC
            $<BUILD_INTERFACE:${VSCP_WINDOWS_PCH_INCLUDE_DIR}>
            $<BUILD_INTERFACE:${VSCP_WINDOWS_HELPERS_INCLUDE_DIR}>)
        target_link_libraries(${target} PUBLIC ws2_32)
        if(TARGET dlfcn-win32::dl)
            target_link_libraries(${target} PUBLIC dlfcn-win32::dl)
        endif()
    endif()

    target_compile_definitions(${target} PUBLIC ${VSCP_OPENSSL_API_DEF})
endfunction()

# =====================================================================
# vscp_sockettcp — isolated TCP socket utility library
# =====================================================================
add_library(vscp_sockettcp STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common/sockettcp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common/vscpmd5.c
)
target_include_directories(vscp_sockettcp
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/common>
        $<BUILD_INTERFACE:${OPENSSL_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include/vscp/util>
)
target_compile_definitions(vscp_sockettcp PUBLIC ${VSCP_OPENSSL_API_DEF})
target_link_libraries(vscp_sockettcp
    PUBLIC
        OpenSSL::SSL
        OpenSSL::Crypto
        Threads::Threads
)
if(WIN32)
    target_include_directories(vscp_sockettcp PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/common/windows>
        $<BUILD_INTERFACE:${VSCP_WINDOWS_PCH_INCLUDE_DIR}>)
    target_link_libraries(vscp_sockettcp PUBLIC ws2_32)
    if(VSCP_PTHREAD_TARGET)
        target_link_libraries(vscp_sockettcp PUBLIC ${VSCP_PTHREAD_TARGET})
    endif()
endif()

# =====================================================================
# vscp_util — low-level platform utility library
# =====================================================================
add_library(vscp_util STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common/vscpbase64.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common/crc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common/crc8.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common/vscp-aes.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common/randpassword.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common/configfile.cpp
    $<$<BOOL:WIN32>:${VSCP_WINDOWS_GETOPT_SOURCE}>
)
target_include_directories(vscp_util
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/common>
        $<BUILD_INTERFACE:${OPENSSL_INCLUDE_DIR}>
        $<INSTALL_INTERFACE:include/vscp/util>
)
target_compile_definitions(vscp_util PUBLIC ${VSCP_OPENSSL_API_DEF})
target_link_libraries(vscp_util
    PUBLIC
        vscp_sockettcp
        OpenSSL::SSL
        OpenSSL::Crypto
        Threads::Threads
)
if(WIN32)
    target_include_directories(vscp_util PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/common/windows>
        $<BUILD_INTERFACE:${VSCP_WINDOWS_GETOPT_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${VSCP_WINDOWS_PCH_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${VSCP_WINDOWS_HELPERS_INCLUDE_DIR}>)
    target_link_libraries(vscp_util PUBLIC ws2_32)
    if(VSCP_PTHREAD_TARGET)
        target_link_libraries(vscp_util PUBLIC ${VSCP_PTHREAD_TARGET})
    endif()
endif()

# =====================================================================
# Split protocol/component libraries
# =====================================================================
add_library(vscp_guid STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vscp/common/guid.cpp
)
vscp_setup_protocol_target(vscp_guid)
target_link_libraries(vscp_guid PUBLIC vscp_util)

add_library(vscp_guidparser STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vscp/common/vscp-guid-parser.c
)
vscp_setup_protocol_target(vscp_guidparser)
target_link_libraries(vscp_guidparser PUBLIC vscp_util)

add_library(vscp_mdf STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vscp/common/mdf.cpp
)
vscp_setup_protocol_target(vscp_mdf)
target_link_libraries(vscp_mdf
    PUBLIC
        vscp_util
        vscp_guid
        vscp_guidparser
        ${EXPAT_LIBRARIES}
        ${CURL_LIBRARIES}
)

# All non-connection protocol sources
set(VSCP_CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vscp/common/vscphelper.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vscp/common/vscpdatetime.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vscp/common/vscpremotetcpif.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vscp/common/vscpcanaldeviceif.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vscp/common/canal-xmlconfig.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vscp/common/register.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vscp/common/userlist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vscp/common/clientlist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vscp/common/interfacelist.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vscp/common/hlo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vscp/common/vscpunit.cpp
)

add_library(vscp_core STATIC ${VSCP_CORE_SOURCES})
vscp_setup_protocol_target(vscp_core)
target_link_libraries(vscp_core
    PUBLIC
        vscp_util
        vscp_guid
        vscp_guidparser
        vscp_mdf
        ${EXPAT_LIBRARIES}
        ${MOSQUITTO_LIBRARIES}
        ${CURL_LIBRARIES}
)
if(UNIX AND NOT APPLE)
    target_link_libraries(vscp_core PUBLIC m dl)
    target_compile_definitions(vscp_core PUBLIC WITH_SYSTEMD)
elseif(APPLE)
    target_link_libraries(vscp_core PUBLIC m dl)
endif()

# Dedicated client connection libraries
add_library(vscp_client_base STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vscp/common/vscp-client-base.cpp
)
vscp_setup_protocol_target(vscp_client_base)
target_link_libraries(vscp_client_base PUBLIC vscp_core)

add_library(vscp_client_canal STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vscp/common/vscp-client-canal.cpp
)
vscp_setup_protocol_target(vscp_client_canal)
target_link_libraries(vscp_client_canal PUBLIC vscp_client_base vscp_core)

add_library(vscp_client_tcp STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vscp/common/vscp-client-tcp.cpp
)
vscp_setup_protocol_target(vscp_client_tcp)
target_link_libraries(vscp_client_tcp PUBLIC vscp_client_base vscp_core)

add_library(vscp_client_udp STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vscp/common/vscp-client-udp.cpp
)
vscp_setup_protocol_target(vscp_client_udp)
target_link_libraries(vscp_client_udp PUBLIC vscp_client_base vscp_core)

add_library(vscp_client_ws1 STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vscp/common/vscp-client-ws1.cpp
)
vscp_setup_protocol_target(vscp_client_ws1)
target_link_libraries(vscp_client_ws1 PUBLIC vscp_client_base vscp_core)

add_library(vscp_client_ws2 STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vscp/common/vscp-client-ws2.cpp
)
vscp_setup_protocol_target(vscp_client_ws2)
target_link_libraries(vscp_client_ws2 PUBLIC vscp_client_base vscp_core)

add_library(vscp_client_multicast STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vscp/common/vscp-client-multicast.cpp
)
vscp_setup_protocol_target(vscp_client_multicast)
target_link_libraries(vscp_client_multicast PUBLIC vscp_client_base vscp_core)

add_library(vscp_client_mqtt STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vscp/common/vscp-client-mqtt.cpp
)
vscp_setup_protocol_target(vscp_client_mqtt)
target_link_libraries(vscp_client_mqtt
    PUBLIC
        vscp_client_base
        vscp_core
        ${MOSQUITTO_LIBRARIES}
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_library(vscp_client_socketcan STATIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/vscp/common/vscp-client-socketcan.cpp
    )
    vscp_setup_protocol_target(vscp_client_socketcan)
    target_link_libraries(vscp_client_socketcan PUBLIC vscp_client_base vscp_core)
endif()

# =====================================================================
# vscp_common — compatibility aggregate library
# =====================================================================
add_library(vscp_common STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/vscp/common/vscp-common.cpp
)
vscp_setup_protocol_target(vscp_common)
target_link_libraries(vscp_common
    PUBLIC
        vscp_core
        vscp_client_base
        vscp_client_canal
        vscp_client_tcp
        vscp_client_udp
        vscp_client_ws1
        vscp_client_ws2
        vscp_client_multicast
        vscp_client_mqtt
)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(vscp_common PUBLIC vscp_client_socketcan)
endif()

# =====================================================================
# CMake package config export (only when installing)
# =====================================================================
if(VSCP_INSTALL)
    set(VSCP_INSTALL_TARGETS
        vscp_common
        vscp_core
        vscp_guid
        vscp_guidparser
        vscp_mdf
        vscp_client_base
        vscp_client_canal
        vscp_client_tcp
        vscp_client_udp
        vscp_client_ws1
        vscp_client_ws2
        vscp_client_multicast
        vscp_client_mqtt
        vscp_sockettcp
        vscp_util
    )
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        list(APPEND VSCP_INSTALL_TARGETS vscp_client_socketcan)
    endif()

    install(TARGETS ${VSCP_INSTALL_TARGETS}
        EXPORT VSCPTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/vscp/common/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vscp
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
    )
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/common/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vscp/util
        FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
    )

    install(DIRECTORY ${VSCP_NLOHMANN_INCLUDE_DIR}/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vscp/third-party/nlohmann
        FILES_MATCHING PATTERN "*.hpp"
    )

    install(EXPORT VSCPTargets
        FILE VSCPTargets.cmake
        NAMESPACE VSCP::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VSCP
    )

    include(CMakePackageConfigHelpers)
    configure_package_config_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/VSCPConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/VSCPConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VSCP
    )
    write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/VSCPConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY AnyNewerVersion
    )
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/VSCPConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/VSCPConfigVersion.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VSCP
    )
    # Also install the custom Find modules so consumers can use them
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VSCP/modules
        FILES_MATCHING PATTERN "*.cmake"
    )
endif()

# =====================================================================
# Tests (opt-in; tests/ directory is the primary test build entry point)
# =====================================================================
if(VSCP_BUILD_TESTS)
    add_subdirectory(tests)
endif()
