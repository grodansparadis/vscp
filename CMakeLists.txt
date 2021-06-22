# CMakeLists.txt
#
# Build instructions for the VSCP daemon.
#
# Copyright (c) 2000-2021 Åke Hedman, the VSCP project
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

cmake_minimum_required(VERSION 3.5)
project(vscpd LANGUAGES CXX C)

# Version settings
if(NOT DEFINED MAJOR_VERSION)
    SET(MAJOR_VERSION "15")
endif()
if(NOT DEFINED MINOR_VERSION)
    SET(MINOR_VERSION "0")
endif()
if(NOT DEFINED PATCH_VERSION)
    SET(PATCH_VERSION "0")
endif()

# Disable abi warnings (nlohmann)
SET(GCC_VSCP_COMPILE_FLAGS "-Wno-psabi")
SET(GCC_VSCP_LINK_FLAGS    "-lgcov")

SET(CPACK_PACKAGE_DESCRIPTION "the VSCP daemon")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A daemon that act as a hub and logic controller for a VSCP based system. It is"
                                      "possible to add drivers to physical devices, interface external nodes over"
                                      "tcp/ip, websockets and to perform other higher end functionality.")
SET(CPACK_PACKAGE_VENDOR "the VSCP project")
SET(CPACK_PACKAGE_CONTACT "info@vscp.org")
set(CPACK_PACKAGE_NAME ${APP_LOW_NAME})
set(CPACK_PACKAGE_INSTALL_REGISTRY_KEY ${APP_NAME})
SET(CPACK_PACKAGE_VERSION_MAJOR "${MAJOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_MINOR "${MINOR_VERSION}")
SET(CPACK_PACKAGE_VERSION_PATCH "${PATCH_VERSION}")
SET(CPACK_PACKAGING_INSTALL_PREFIX "/")
SET(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${MAJOR_VERSION}.${MINOR_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}")
SET(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${MAJOR_VERSION}.${MINOR_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}")
#SET(CPACK_IGNORE_FILES "\\.psd$;/CVS/;/\\.svn/;/\\.git/;\\.swp$;/CMakeLists.txt.user;\\.#;/#;\\.tar.gz$;/CMakeFiles/;CMakeCache.txt;\\.qm$;/build/;\\.diff$;.DS_Store'")
set(CPACK_SOURCE_IGNORE_FILES ${CPACK_IGNORE_FILES})
SET(CPACK_NSIS_HELP_LINK ${APP_URL})
SET(CPACK_NSIS_URL_INFO_ABOUT ${APP_URL})
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")


if(NOT DEFINED APP_LOW_NAME)
set(APP_LOW_NAME "vscpd")
endif()

if(NOT DEFINED APP_NAME)
set(APP_NAME "VSCPD")
endif()

if(NOT DEFINED APP_URL)
set(APP_URL "https://github.com/grodansparadis/vscp")
endif()

if(NOT DEFINED APP_VERSION)
set(APP_VERSION, "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}")
endif()

# Man build and install
if(NOT DEFINED WIN32)
    add_subdirectory(man)
    add_subdirectory(src/vscp/vscp-mkpasswd)
endif()

option(USE_SSL "Use SSL" TRUE)

## --- C++11 build flags ---
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

# Build locations
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

# Generate position-independent code (-fPIC on UNIX)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# --- System Libraries ---
include(GNUInstallDirs)

INCLUDE(InstallRequiredSystemLibraries)

# Disable abi warnings (nlohmann)
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_VSCP_COMPILE_FLAGS}")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_VSCP_LINK_FLAGS}")

#   * * * * CPACK * * * *

if(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")

    set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA "${CMAKE_CURRENT_SOURCE_DIR}/resources/debpkg/folders;"
                                           "${CMAKE_CURRENT_SOURCE_DIR}/resources/debpkg/conffiles;"
                                           "${CMAKE_CURRENT_SOURCE_DIR}/resources/debpkg/postinst;"
                                           "${CMAKE_CURRENT_SOURCE_DIR}/resources/debpkg/logrotate;"
                                           "${CMAKE_CURRENT_SOURCE_DIR}/resources/debpkg/prerm;"
                                           "${CMAKE_CURRENT_SOURCE_DIR}/resources/debpkg/postrm;" )

    IF(WIN32)
        SET(CPACK_GENERATOR "NSIS" "ZIP")
    ELSE(WIN32)
        SET(CPACK_GENERATOR "DEB" "RPM" "TGZ" "ZIP")
    ENDIF(WIN32)

    SET(CPACK_COMPONENTS_ALL Libraries ApplicationData)

    # Configure file with custom definitions for NSIS.
    if(WIN32 AND NOT UNIX) 
        configure_file(
            ${PROJECT_SOURCE_DIR}/resources/nsis/NSIS.definitions.nsh.in
            ${CMAKE_CURRENT_BINARY_DIR}/resources/nsis/NSIS.definitions.nsh
        )
    else()
        SET(CPACK_DEBIAN_PACKAGE_DEPENDS 
                "libuuid1, lsb-base (>=4.1+Debian3), libc6-dev (>= 2.14), libstdc++6 (>= 5.2), libgcc1 (>= 1:3.0), libssl-dev (>=1.0.0), libmosquitto-dev (>=1.4.15), libexpat1-dev (>=2.0), libmosquitto-dev, libsystemd-dev, libwrap0-dev (>=7.6), libz-dev")
        SET(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
        SET(CPACK_DEBIAN_PACKAGE_SECTION "network")
        SET(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})
        set(CPACK_STRIP_FILES "bin/vscpd")
        set(CPACK_SOURCE_STRIP_FILES "")      
    endif()

    set(CPACK_PACKAGE_EXECUTABLES "vscpd")
 
    # Load packaging facilities.
    include(CPack)

endif(EXISTS "${CMAKE_ROOT}/Modules/CPack.cmake")

if(WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DWIN32_LEAN_AND_MEANS)
    add_definitions(-DOPENSSL_API_1_1)
    add_definitions(-D_POSIX_C_SOURCE)    
    add_definitions(-DVCPKG_TARGET_TRIPLET=x64-windows)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    set(LIBS_SYSTEM ws2_32)
    if (MSVC)
        #     # use precompiled headers
        #     set (CMAKE_AUTOMOC_MOC_OPTIONS "-bstdafx.h" "-fstdafx.h")
        set (CMAKE_AUTOMOC_MOC_OPTIONS "-bstdafx.h ")
    endif()    
elseif(UNIX)
    set(LIBS_SYSTEM c stdc++)
endif()


# set the module path
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

# pthreads
set (THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# openssl
find_package(OpenSSL REQUIRED)

# Windows Libs
if (WIN32)
    # pthread
    find_package(pthread REQUIRED)

    # SQLite3
    #find_package(unofficial-sqlite3 CONFIG REQUIRED)

    # Mosquitto lib
    find_package(mosquitto CONFIG REQUIRED)

    # Expat C library
    find_package(expat CONFIG REQUIRED)

    # dlfcn
    find_package(dlfcn-win32 CONFIG REQUIRED)

    # Logging
    find_package(spdlog CONFIG REQUIRED)
else()
    # expat
    include(FindEXPAT)
    find_package(EXPAT REQUIRED)
    include_directories(${EXPAT_INCLUDE_DIRS})

    # Mosquitto lib
    include(FindMosquitto)

    # Logger
    #find_package(spdlog CONFIG REQUIRED)
    
endif(WIN32)

# Set civetweb build flags
add_definitions(-DUSE_WEBSOCKET)
add_definitions(-DUSE_IPV6)
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DUSE_IPV6 -DUSE_WEBSOCKET ")
string(FIND "${OPENSSL_VERSION}" "1.1." SSL_VER_POS)
if (${SSL_VER_POS})
    #set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DOPENSSL_API_1_0")
    add_definitions(-DOPENSSL_API_1_0)
else()
    #set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DOPENSSL_API_1_1")
    add_definitions(-DOPENSSL_API_1_1)
endif()

if (WIN32)
    add_executable(vscpd
        src/vscp/daemon/vscpd.cpp
        src/vscp/daemon/controlobject.h
        src/vscp/daemon/controlobject.cpp
        src/vscp/daemon/devicethread.h
        src/vscp/daemon/devicethread.cpp
        src/vscp/daemon/devicelist.h
        src/vscp/daemon/devicelist.cpp
        src/vscp/common/version.h
        src/vscp/common/vscp.h
        src/vscp/common/vscpremotetcpif.h
        src/vscp/common/vscpremotetcpif.cpp
        src/vscp/common/vscpdatetime.h
        src/vscp/common/vscpdatetime.cpp
        src/vscp/common/guid.h
        src/vscp/common/guid.cpp
        src/vscp/common/mdf.h
        src/vscp/common/mdf.cpp
        src/vscp/common/vscphelper.h
        src/vscp/common/vscphelper.cpp
        src/vscp/common/canal.h
        src/vscp/common/canaldlldef.h
        src/vscp/common/vscpcanaldeviceif.h
        src/vscp/common/vscpcanaldeviceif.cpp
        src/vscp/common/canal_xmlconfig.h
        src/vscp/common/canal_xmlconfig.cpp
        src/vscp/common/register.h
        src/vscp/common/register.cpp
        src/common/third_party/mustache.hpp
        src/common/third_party/civetweb-1.14/include/civetweb.h
        src/common/third_party/civetweb-1.14/src/civetweb.c
        src/common/third_party/sqlite3-3.35.4/sqlite3.h
        src/common/third_party/sqlite3-3.35.4/sqlite3.c
        src/common/windows/getopt.c
        src/common/configfile.h
        src/common/configfile.cpp
        src/common/sockettcp.h
        src/common/sockettcp.c
        src/common/vscpbase64.h
        src/common/vscpbase64.c
        src/common/vscp_aes.h
        src/common/vscp_aes.c
        src/common/crc.h
        src/common/crc.c
        src/common/crc8.h
        src/common/crc8.c
        src/common/vscpmd5.h
        src/common/vscpmd5.c
        src/common/fastpbkdf2.h
        src/common/fastpbkdf2.c  
        src/common/randpassword.h
        src/common/randpassword.cpp     
    )
else()
    add_executable(vscpd
        src/vscp/daemon/vscpd.cpp
        src/vscp/daemon/controlobject.h
        src/vscp/daemon/controlobject.cpp
        src/vscp/daemon/devicethread.h
        src/vscp/daemon/devicethread.cpp
        src/vscp/daemon/devicelist.h
        src/vscp/daemon/devicelist.cpp
        src/vscp/common/version.h
        src/vscp/common/vscp.h
        src/vscp/common/vscpremotetcpif.h
        src/vscp/common/vscpremotetcpif.cpp
        src/vscp/common/vscpdatetime.h
        src/vscp/common/vscpdatetime.cpp
        src/vscp/common/guid.h
        src/vscp/common/guid.cpp
        src/vscp/common/mdf.h
        src/vscp/common/mdf.cpp
        src/vscp/common/vscphelper.h
        src/vscp/common/vscphelper.cpp
        src/vscp/common/canal.h
        src/vscp/common/canaldlldef.h
        src/vscp/common/vscpcanaldeviceif.h
        src/vscp/common/vscpcanaldeviceif.cpp
        src/vscp/common/canal_xmlconfig.h
        src/vscp/common/canal_xmlconfig.cpp
        src/vscp/common/register.h
        src/vscp/common/register.cpp
        src/vscp/common/vscp_client_base.h
        src/vscp/common/vscp_client_base.cpp
        src/vscp/common/vscp_client_mqtt.h
        src/vscp/common/vscp_client_mqtt.cpp
        src/common/third_party/mustache.hpp
        src/common/third_party/civetweb-1.14/include/civetweb.h
        src/common/third_party/civetweb-1.14/src/civetweb.c
        src/common/third_party/sqlite3-3.35.4/sqlite3.h
        src/common/third_party/sqlite3-3.35.4/sqlite3.c
        src/common/configfile.h
        src/common/configfile.cpp
        src/common/sockettcp.h
        src/common/sockettcp.c
        src/common/vscpbase64.h
        src/common/vscpbase64.c
        src/common/vscp_aes.h
        src/common/vscp_aes.c
        src/common/crc.h
        src/common/crc.c
        src/common/crc8.h
        src/common/crc8.c
        src/common/vscpmd5.h
        src/common/vscpmd5.c
        src/common/fastpbkdf2.h
        src/common/fastpbkdf2.c  
        src/common/randpassword.h
        src/common/randpassword.cpp     
    )
 endif() 

#     * * * Includes * * *
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (WIN32)
    #find_path(PTHREAD_INCLUDE_DIR pthread.h)
    include_directories( PUBLIC ./src/vscp/daemon
                            ./src/common 
                            ./src/vscp/common/  
                            ./src/common/third_party
                            ./src/common/third_party/nlohmann
                            ./src/common/third_party/civetweb-1.14/
                            ./src/common/third_party/civetweb-1.14/include/
                            ./src/common/third_party/sqlite3-3.35.4/
                            ./src/common/third_party/spdlog-1.8.5/include/
                            ./src/common/windows
                            ${PThreads4W_INCLUDE_DIR}
                            ${CMAKE_BINARY_DIR}
                            ${OPENSSL_INCLUDE_DIR}
                            ${MOSQUITTO_INCLUDE_DIRS}
                            ${EXPAT_INCLUDE_DIRS}
                            ${DLFCN_INCLUDES}
                            ${LOCAL_WIN_INCLUDE} )
else()
    include_directories( PUBLIC ./src/vscp/daemon
                            ./src/common 
                            ./src/vscp/common/  
                            ./src/common/third_party
                            ./src/common/third_party/nlohmann
                            ./src/common/third_party/civetweb-1.14/
                            ./src/common/third_party/civetweb-1.14/include/
                            ./src/common/third_party/sqlite3-3.35.4/
                            ./src/common/third_party/spdlog-1.8.5/include/
                            ${PThreads4W_INCLUDE_DIR}
                            ${CMAKE_BINARY_DIR}
                            ${OPENSSL_INCLUDE_DIR}
                            ${MOSQUITTO_INCLUDE_DIRS}
                            ${EXPAT_INCLUDE_DIRS}
                            ${DLFCN_INCLUDES}
                            ${LOCAL_WIN_INCLUDE} )
 endif()   

if(WIN32)
    target_link_libraries(vscpd PRIVATE 
        netapi32.lib                   
        PThreads4W::PThreads4W_CXXEXC
        OpenSSL::SSL 
        OpenSSL::Crypto
        OpenSSL::SSL OpenSSL::Crypto
        ${MOSQUITTO_LIBRARIES}
        expat::expat
        dlfcn-win32::dl
        spdlog::spdlog spdlog::spdlog_header_only
    )
else()
    target_link_libraries(vscpd PRIVATE     
        m
        dl
        systemd
        Threads::Threads
        OpenSSL::SSL 
        OpenSSL::Crypto
        ${OPENSSL_LIBRARIES}
        ${MOSQUITTO_LIBRARIES}
        ${EXPAT_LIBRARIES}
    )
endif()

if(WIN32)
else()

# Build main directory structure
install(DIRECTORY 
            DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/lib/vscp
            DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
install(DIRECTORY 
            DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/lib/vscp/drivers/level1/
            DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
install(DIRECTORY 
            DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/lib/vscp/drivers/level2/
            DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
install(DIRECTORY 
            DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/lib/vscp/web/html/
            DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
install(DIRECTORY 
            DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/lib/vscp/web/html/js/
            DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
install(DIRECTORY 
            DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/lib/vscp/web/html/css/
            DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
install(DIRECTORY
            DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/lib/vscp/web/html/images/
            DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)

# Location for log
install(DIRECTORY 
            DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/log/vscp/
            DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
            
# The VSCP daemon executable
install(TARGETS vscpd
        RUNTIME DESTINATION 	
        ${CMAKE_INSTALL_SBINDIR})

# Password generation
install(TARGETS vscp-mkpasswd
        RUNTIME DESTINATION 	
        ${CMAKE_INSTALL_BINDIR})

# The VSCP daemon systemctl service file
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/linux/vscpd.service
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/systemd/system/)

# Default configuration file
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/linux/vscpd.json
            DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/vscp/
            PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ)
# Default encryption key
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/vscp.key
            DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/vscp/
            PERMISSIONS OWNER_READ OWNER_WRITE)
# Readme for certs
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/README-certs
            DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/vscp/certs/
            PERMISSIONS OWNER_READ OWNER_WRITE)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/README-ca-certificates
            DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/vscp/ca-certificates/
            PERMISSIONS OWNER_READ OWNER_WRITE)
# Database
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/database/vscp_events.sqlite3
            DESTINATION ${CMAKE_INSTALL_LOCALSTATEDIR}/lib/vscp/vscpd/
            PERMISSIONS OWNER_READ OWNER_WRITE)

# Scripts
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/resources/linux/vscp_eth_to_guid
              ${CMAKE_CURRENT_SOURCE_DIR}/resources/linux/download-event-db
DESTINATION ${CMAKE_INSTALL_BINDIR}
PERMISSIONS OWNER_READ OWNER_WRITE)

#SET(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
#            "${CMAKE_CURRENT_SOURCE_DIR}/prerm;${CMAKE_CURRENT_SOURCE_DIR}/postrm;${CMAKE_CURRENT_SOURCE_DIR}/files")
# enable start/restart vscpd service
#install(CODE "execute_process(COMMAND systemctl stop vscpd && systemctl enable vscpd && service vscpd start)") 

endif()