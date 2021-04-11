# CMakeLists.txt
#
# Build instructions for the VSCP daemon.
#
# Copyright (c) 2000-2021 Åke Hedman, Grodans Paradis AB
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
# VSCP_PATH - set to path to local vscp repository. Default is "../vscp". Set with
# -DVSCP_PATH="path"
#

cmake_minimum_required(VERSION 3.5)
project(vscpd LANGUAGES CXX C)

option(USE_SSL "Use SSL" TRUE)

## --- C++11 build flags ---
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generate position-independent code (-fPIC on UNIX)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# --- System Libraries ---
include(GNUInstallDirs)

if(WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DWIN32_LEAN_AND_MEANS)
    add_definitions(-DOPENSSL_API_1_1)
    add_definitions(-D_POSIX_C_SOURCE)    
    add_definitions(-DVCPKG_TARGET_TRIPLET=x64-windows)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    set(LIBS_SYSTEM ws2_32)
elseif(UNIX)
    set(LIBS_SYSTEM c stdc++)
endif()

if (MSVC)
#     # use precompiled headers
#     set (CMAKE_AUTOMOC_MOC_OPTIONS "-bstdafx.h" "-fstdafx.h")
    set (CMAKE_AUTOMOC_MOC_OPTIONS "-bstdafx.h ")
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# set the module path
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

# pthreads
set (THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# openssl
find_package(OpenSSL REQUIRED)

# Windows Libs
if (WIN32)
    # Paho MQTT
    find_package(pthread REQUIRED)
    #find_package(eclipse-paho-mqtt-c CONFIG REQUIRED)
    #get_target_property(PAHO_INCLUDE_DIR eclipse-paho-mqtt-c::paho-mqtt3cs INTERFACE_INCLUDE_DIRECTORIES)
    #get_target_property(PAHO_LIBRARIES eclipse-paho-mqtt-c::paho-mqtt3cs INTERFACE_LINK_LIBRARIES)

    # SQLite3
    #find_package(unofficial-sqlite3 CONFIG REQUIRED)

    # Mosquitto lib
    find_package(mosquitto REQUIRED)

    # Expat C library
    find_package(expat CONFIG REQUIRED)

    # dlfcn
    find_package(dlfcn-win32 CONFIG REQUIRED)
else()
    # expat
    include(FindEXPAT)
    find_package(EXPAT REQUIRED)
    include_directories(${EXPAT_INCLUDE_DIRS})

    # Mosquitto lib
    include(FindMosquitto)
    #find_package(mosquitto REQUIRED)
    # paho
    #include(Findpaho-mqtt)
    #find_package(paho-mqtt REQUIRED)
endif()

# Set civetweb build flags
add_definitions(-DUSE_WEBSOCKET)
add_definitions(-DUSE_IPV6)
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DUSE_IPV6 -DUSE_WEBSOCKET ")
string(FIND "${OPENSSL_VERSION}" "1.1." SSL_VER_POS)
if(${SSL_VER_POS})
    #set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DOPENSSL_API_1_0")
    add_definitions(-DOPENSSL_API_1_0)
else()
    #set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DOPENSSL_API_1_1")
    add_definitions(-DOPENSSL_API_1_1)
endif()

add_executable(vscpd
    src/vscp/daemon/linux/vscpd.cpp
    src/vscp/common/version.h
    src/vscp/common/vscp.h
    src/vscp/common/controlobject.h
    src/vscp/common/controlobject.cpp
    src/vscp/common/devicethread.h
    src/vscp/common/devicethread.cpp
    src/vscp/common/devicelist.h
    src/vscp/common/devicelist.cpp
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

if (WIN32)
    #find_path(PTHREAD_INCLUDE_DIR pthread.h)
    include_directories( PUBLIC ./src/common 
                            ./src/vscp/common/  
                            ./src/common/third_party
                            ./src/common/third_party/nlohmann
                            ./src/common/third_party/civetweb-1.14/include
                            ${PThreads4W_INCLUDE_DIR}
                            ${CMAKE_BINARY_DIR}
                            ${OPENSSL_INCLUDE_DIR}
                            ${MOSQUITTO_INCLUDE_DIRS}
                            ${EXPAT_INCLUDE_DIRS}
                            ${DLFCN_INCLUDES}
                            ${LOCAL_WIN_INCLUDE} )
else()
    include_directories( PUBLIC ./src/common 
                            ./src/vscp/common/  
                            ./src/common/third_party
                            ./src/common/third_party/nlohmann
                            ./src/common/third_party/civetweb-1.14/include
                            ${PThreads4W_INCLUDE_DIR}
                            ${CMAKE_BINARY_DIR}
                            ${OPENSSL_INCLUDE_DIR}
                            ${MOSQUITTO_INCLUDE_DIRS}
                            ${EXPAT_INCLUDE_DIRS}
                            ${DLFCN_INCLUDES}
                            ${LOCAL_WIN_INCLUDE} )
 endif()   

if (WIN32)

    target_link_libraries(vscpd PRIVATE     
        PThreads4W::PThreads4W_CXXEXC
        OpenSSL::SSL 
        OpenSSL::Crypto
        OpenSSL::SSL OpenSSL::Crypto
        ${MOSQUITTO_LIBRARIES}
        expat::expat
        dlfcn-win32::dl)
    else()
    target_link_libraries(vscpd PRIVATE     
        m
        dl
        Threads::Threads
        OpenSSL::SSL 
        OpenSSL::Crypto
        ${OPENSSL_LIBRARIES}
        ${MOSQUITTO_LIBRARIES}
        ${EXPAT_LIBRARIES}
        ${DLFCN_LIBS})
endif()


install(TARGETS vscpd
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})


