# CMakeLists.txt
#
# Build instructions for VSCP Works.
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
project(vscp-works-qt LANGUAGES CXX C)

option(USE_SSL "Use SSL" TRUE)

## --- C++11 build flags ---
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generate position-independent code (-fPIC on UNIX)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# --- System Libraries ---
include(GNUInstallDirs)

# --- Path to vscp --- 
if (NOT DEFINED VSCP_PATH)
set (VSCP_PATH "${PROJECT_SOURCE_DIR}/../vscp")
endif()

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

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# set the module path
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

# qt
#find_package(Qt5 COMPONENTS Widgets LinguistTools REQUIRED Core Network Quick Sql Qml SerialPort)
find_package(Qt5 COMPONENTS Widgets LinguistTools REQUIRED Core Network Quick Sql Charts)

# pthreads
set (THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

#openssl
find_package(OpenSSL REQUIRED)

set(TS_FILES vscp-works-qt_en_150.ts)

# Windows Libs
if (WIN32)
    # Paho MQTT
    find_package(pthread REQUIRED)
    find_package(eclipse-paho-mqtt-c CONFIG REQUIRED)
    get_target_property(PAHO_INCLUDE_DIR eclipse-paho-mqtt-c::paho-mqtt3cs INTERFACE_INCLUDE_DIRECTORIES)
    get_target_property(PAHO_LIBRARIES eclipse-paho-mqtt-c::paho-mqtt3cs INTERFACE_LINK_LIBRARIES)

    # SQLite3
    find_package(unofficial-sqlite3 CONFIG REQUIRED)

    # Expat C library
    find_package(expat CONFIG REQUIRED)

    # dlfcn
    find_package(dlfcn-win32 CONFIG REQUIRED)
else()
    # expat
    include(FindEXPAT)
    find_package(EXPAT REQUIRED)
    include_directories(${EXPAT_INCLUDE_DIRS})
    # paho
    include(Findpaho-mqtt)
    find_package(paho-mqtt REQUIRED)
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

# set(UI )
qt5_wrap_ui(UI src/mainwindow.ui    
    src/cdlgmainsettings.ui
    src/cdlgconnsettingslocal.ui
    src/cdlgconnsettingscanal.ui
    src/cdlgconnsettingstcpip.ui
    src/cdlgconnsettingssocketcan.ui
    src/cdlgconnsettingsmqtt.ui
    src/cdlgconnsettingsws1.ui
    src/cdlgconnsettingsws2.ui
    src/cdlgconnsettingsudp.ui
    src/cdlgconnsettingsmulticast.ui
    src/cdlgconnsettingsrest.ui
    src/cdlgconnsettingsrawcan.ui
    src/cdlgconnsettingsrawmqtt.ui
    src/cdlgnewconnection.ui
    src/cdlglevel1filter.ui
    src/cdlglevel1filterwizard.ui
    src/cdlglevel2filter.ui
    src/cdlgsocketcanflags.ui
    src/cdlgmqttpublish.ui
    src/cdlgmqttsubscribe.ui
    src/cdlgknownguid.ui
    src/cdlgeditguid.ui
    src/cdlgcanfilter.ui
    src/cdlgtls.ui
    src/cdlglog.ui
    str/cdlgsensorindex.ui
    src/cdlgeditsensorindex.ui
    src/cdlgtxedit.ui
    src/cdlgsessionfilter.ui
    src/cdlgeditsessionfilter.ui
    src/cdlgselectclass.ui
    src/cdlgselectguid.ui
    src/cdlgselectobid.ui
    src/cdlgselectdata.ui
    src/cdlgselectdatasize.ui
    src/cdlgselectmeasurementvalue.ui
    src/cdlgselectpriority.ui
    src/cdlgselectmeasurementvalue.ui
    src/cdlgselecttimestamp.ui
    src/cdlgselectunit.ui
    src/cdlgselectsensorindex.ui
    src/cdlgselectdatacoding.ui
    src/cdlgselectdate.ui
    src/cdlgselectconstraint.ui
)

add_executable(vscp-works-qt
    src/vscpworks.qrc
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/filedownloader.h
    src/filedownloader.cpp
    build/ui_cdlgnewconnection.h
    src/cdlgnewconnection.h
    src/cdlgnewconnection.cpp
    build/ui_cdlgmainsettings.h
    src/cdlgmainsettings.h
    src/cdlgmainsettings.cpp

    build/ui_cdlgconnsettingslocal.h
    src/cdlgconnsettingslocal.h
    src/cdlgconnsettingslocal.cpp

    build/ui_cdlgconnsettingscanal.h
    src/cdlgconnsettingscanal.h
    src/cdlgconnsettingscanal.cpp

    build/ui_cdlgconnsettingstcpip.h
    src/cdlgconnsettingstcpip.h
    src/cdlgconnsettingstcpip.cpp

    build/ui_cdlgconnsettingssocketcan.h
    src/cdlgconnsettingssocketcan.h
    src/cdlgconnsettingssocketcan.cpp

    build/ui_cdlgsocketcanflags.h
    src/cdlgsocketcanflags.h
    src/cdlgsocketcanflags.cpp

    build/ui_cdlgconnsettingsmqtt.h
    src/cdlgconnsettingsmqtt.h
    src/cdlgconnsettingsmqtt.cpp

    build/ui_cdlgmqttpublish.h
    src/cdlgmqttpublish.h
    src/cdlgmqttpublish.cpp

    build/ui_cdlgmqttsubscribe.h
    src/cdlgmqttsubscribe.h
    src/cdlgmqttsubscribe.cpp

    build/ui_cdlgconnsettingsws1.h
    src/cdlgconnsettingsws1.h
    src/cdlgconnsettingsws1.cpp

    build/ui_cdlgconnsettingsws2.h
    src/cdlgconnsettingsws2.h
    src/cdlgconnsettingsws2.cpp

    build/ui_cdlgconnsettingsudp.h
    src/cdlgconnsettingsudp.h
    src/cdlgconnsettingsudp.cpp

    build/ui_cdlgconnsettingsmulticast.h
    src/cdlgconnsettingsmulticast.h
    src/cdlgconnsettingsmulticast.cpp

    build/ui_cdlgconnsettingsrawcan.h
    src/cdlgconnsettingsrawcan.h
    src/cdlgconnsettingsrawcan.cpp

    build/ui_cdlgconnsettingsrawmqtt.h
    src/cdlgconnsettingsrawmqtt.h
    src/cdlgconnsettingsrawmqtt.cpp

    build/ui_cdlgconnsettingsrest.h
    src/cdlgconnsettingsrest.h
    src/cdlgconnsettingsrest.cpp

    build/ui_cdlgknownguid.h
    src/cdlgknownguid.h
    src/cdlgknownguid.cpp
    
    src/cdlglevel1filter.h
    src/cdlglevel1filter.cpp
    build/ui_cdlglevel1filter.h

    src/cdlglevel1filterwizard.h
    src/cdlglevel1filterwizard.cpp
    build/ui_cdlglevel1filterwizard.h

    src/cdlglevel2filter.ui
    src/cdlglevel2filter.h
    src/cdlglevel2filter.cpp
    build/ui_cdlglevel2filter.h

    src/cdlgcanfilter.ui
    src/cdlgcanfilter.h
    src/cdlgcanfilter.cpp

    src/cdlgtls.ui
    src/cdlgtls.h
    src/cdlgtls.cpp  

    src/cfrmsession.h
    src/cfrmsession.cpp
    src/vscpworks.h
    src/vscpworks.cpp
    src/canalconfigwizard.h
    src/canalconfigwizard.cpp

    src/eventlistmodel.h
    src/eventlistmodel.cpp

    src/cdlgeditguid.ui
    src/cdlgeditguid.h
    src/cdlgeditguid.cpp

    src/cdlgsensorindex.ui
    src/cdlgsensorindex.h
    src/cdlgsensorindex.cpp

    src/cdlgeditsensorindex.ui
    src/cdlgeditsensorindex.h
    src/cdlgeditsensorindex.cpp

    src/cdlgtxedit.ui
    src/cdlgtxedit.h
    src/cdlgtxedit.cpp

    src/ctxevent.h
    src/ctxevent.cpp

    src/sessionfilter.h
    src/sessionfilter.cpp

    src/cdlgsessionfilter.ui
    src/cdlgsessionfilter.h
    src/cdlgsessionfilter.cpp

    src/cdlgeditsessionfilter.ui
    src/cdlgeditsessionfilter.h
    src/cdlgeditsessionfilter.cpp

    src/cdlgselectclass.ui
    src/cdlgselectclass.h
    src/cdlgselectclass.cpp

    src/cdlgselectguid.ui
    src/cdlgselectguid.h
    src/cdlgselectguid.cpp

    src/cdlgselectobid.ui
    src/cdlgselectobid.h
    src/cdlgselectobid.cpp

    src/cdlgselectdata.ui
    src/cdlgselectdata.h
    src/cdlgselectdata.cpp

    src/cdlgselectdatasize.ui
    src/cdlgselectdatasize.h
    src/cdlgselectdatasize.cpp

    src/cdlgselectdatacoding.ui
    src/cdlgselectdatacoding.h
    src/cdlgselectdatacoding.cpp

    src/cdlgselectmeasurementvalue.ui
    src/cdlgselectmeasurementvalue.h
    src/cdlgselectmeasurementvalue.cpp

    src/cdlgselectpriority.ui
    src/cdlgselectpriority.h
    src/cdlgselectpriority.cpp

    src/cdlgselecttimestamp.ui
    src/cdlgselecttimestamp.h
    src/cdlgselecttimestamp.cpp

    src/cdlgselectunit.ui
    src/cdlgselectunit.h
    src/cdlgselectunit.cpp

    src/cdlgselectsensorindex.ui
    src/cdlgselectsensorindex.h
    src/cdlgselectsensorindex.cpp

    src/cdlgselectdate.ui
    src/cdlgselectdate.h
    src/cdlgselectdate.cpp

    src/cdlgselectconstraint.ui
    src/cdlgselectconstraint.h
    src/cdlgselectconstraint.cpp

    src/widgetgraph.h
    src/widgetgraph.cpp 

    src/xyseries.h
    src/xyseries.cpp

    ${VSCP_PATH}/src/vscp/common/version.h
    ${VSCP_PATH}/src/vscp/common/vscp.h
    ${VSCP_PATH}/src/vscp/common/vscpremotetcpif.h
    ${VSCP_PATH}/src/vscp/common/vscpremotetcpif.cpp
    ${VSCP_PATH}/src/vscp/common/vscpdatetime.h
    ${VSCP_PATH}/src/vscp/common/vscpdatetime.cpp
    ${VSCP_PATH}/src/vscp/common/guid.h
    ${VSCP_PATH}/src/vscp/common/guid.cpp
    ${VSCP_PATH}/src/vscp/common/mdf.h
    ${VSCP_PATH}/src/vscp/common/mdf.cpp
    ${VSCP_PATH}/src/vscp/common/vscphelper.h
    ${VSCP_PATH}/src/vscp/common/vscphelper.cpp
    ${VSCP_PATH}/src/vscp/common/canal.h
    ${VSCP_PATH}/src/vscp/common/canaldlldef.h
    ${VSCP_PATH}/src/vscp/common/vscpcanaldeviceif.h
    ${VSCP_PATH}/src/vscp/common/vscpcanaldeviceif.cpp
    ${VSCP_PATH}/src/vscp/common/canal_xmlconfig.h
    ${VSCP_PATH}/src/vscp/common/canal_xmlconfig.cpp
    ${VSCP_PATH}/src/vscp/common/vscp_client_base.h
    ${VSCP_PATH}/src/vscp/common/vscp_client_base.cpp
    ${VSCP_PATH}/src/vscp/common/vscp_client_local.h
    ${VSCP_PATH}/src/vscp/common/vscp_client_local.cpp
    ${VSCP_PATH}/src/vscp/common/vscp_client_canal.h
    ${VSCP_PATH}/src/vscp/common/vscp_client_canal.cpp    
    ${VSCP_PATH}/src/vscp/common/vscp_client_mqtt.h
    ${VSCP_PATH}/src/vscp/common/vscp_client_mqtt.cpp
    ${VSCP_PATH}/src/vscp/common/vscp_client_tcp.h
    ${VSCP_PATH}/src/vscp/common/vscp_client_tcp.cpp
    ${VSCP_PATH}/src/vscp/common/vscp_client_socketcan.h
    ${VSCP_PATH}/src/vscp/common/vscp_client_socketcan.cpp
    ${VSCP_PATH}/src/vscp/common/vscp_client_ws1.h
    ${VSCP_PATH}/src/vscp/common/vscp_client_ws1.cpp 
    ${VSCP_PATH}/src/vscp/common/vscp_client_ws2.h
    ${VSCP_PATH}/src/vscp/common/vscp_client_ws2.cpp 
    ${VSCP_PATH}/src/vscp/common/vscp_client_udp.h
    ${VSCP_PATH}/src/vscp/common/vscp_client_udp.cpp 
    ${VSCP_PATH}/src/vscp/common/vscp_client_multicast.h
    ${VSCP_PATH}/src/vscp/common/vscp_client_multicast.cpp 
    ${VSCP_PATH}/src/vscp/common/vscp_client_rest.h
    ${VSCP_PATH}/src/vscp/common/vscp_client_rest.cpp
    ${VSCP_PATH}/src/vscp/common/vscp_client_rawcan.h
    ${VSCP_PATH}/src/vscp/common/vscp_client_rawcan.cpp
    ${VSCP_PATH}/src/vscp/common/vscp_client_rawmqtt.h
    ${VSCP_PATH}/src/vscp/common/vscp_client_rawmqtt.cpp
    ${VSCP_PATH}/src/vscp/common/vscp_bootloader.h
    ${VSCP_PATH}/src/vscp/common/vscp_bootloader.cpp
    ${VSCP_PATH}/src/vscp/common/vscpunit.h
    ${VSCP_PATH}/src/vscp/common/vscpunit.cpp
    ${VSCP_PATH}/src/common/third_party/mustache.hpp
    ${VSCP_PATH}/src/common/third_party/civetweb-1.13/include/civetweb.h
    ${VSCP_PATH}/src/common/third_party/civetweb-1.13/src/civetweb.c
    ${VSCP_PATH}/src/common/sockettcp.h
    ${VSCP_PATH}/src/common/sockettcp.c
    ${VSCP_PATH}/src/common/vscpbase64.h
    ${VSCP_PATH}/src/common/vscpbase64.c
    ${VSCP_PATH}/src/common/vscp_aes.h
    ${VSCP_PATH}/src/common/vscp_aes.c
    ${VSCP_PATH}/src/common/crc.h
    ${VSCP_PATH}/src/common/crc.c
    ${VSCP_PATH}/src/common/crc8.h
    ${VSCP_PATH}/src/common/crc8.c
    ${VSCP_PATH}/src/common/vscpmd5.h
    ${VSCP_PATH}/src/common/vscpmd5.c
    ${VSCP_PATH}/src/common/fastpbkdf2.h
    ${VSCP_PATH}/src/common/fastpbkdf2.c    
    install/vscp-works-qt.desktop
    ${TS_FILES}    
  ) 

if (WIN32)
    #find_path(PTHREAD_INCLUDE_DIR pthread.h)

    include_directories( PUBLIC ./src 
        ./build
        ./ui
        ${VSCP_PATH}/src/vscp/common/  
        ${VSCP_PATH}/src/common
        ${VSCP_PATH}/src/common/third_party
        ${VSCP_PATH}/src/common/third_party/nlohmann
        ${VSCP_PATH}/src/common/third_party/civetweb-1.13/include
        ${PThreads4W_INCLUDE_DIR}
        ${Qt5Widgets_INCLUDE_DIRS}
        ${Qt5Core_INCLUDE_DIRS}
        ${Qt5SerialBus_INCLUDE_DIRS}
        ${Qt5SerialPort_INCLUDE_DIRS}
        ${Qt5Network_INCLUDE_DIRS}
        ${CMAKE_BINARY_DIR}
        ${OPENSSL_INCLUDE_DIR}
        ${PAHO_INCLUDE_DIRS}
        ${EXPAT_INCLUDE_DIRS}
        ${DLFCN_INCLUDES}
        ${LOCAL_WIN_INCLUDE}
        ${LOCAL_SYSLOG_INCLUDE} )
else()
    include_directories( PUBLIC ./src 
        ./build
        ./ui
        third_party/mqtt
        third_party/mqtt/clib
        ${VSCP_PATH}/src/vscp/common/
        ${VSCP_PATH}/src/common
        ${VSCP_PATH}/src/common/third_party
        ${VSCP_PATH}/src/common/third_party/nlohmann
        ${VSCP_PATH}/src/common/third_party/civetweb-1.13/include
        ${Qt5Core_INCLUDE_DIRS}
        ${Qt5SerialBus_INCLUDE_DIRS}
        ${Qt5SerialPort_INCLUDE_DIRS}
        ${Qt5Network_INCLUDE_DIRS}
        ${CMAKE_BINARY_DIR}
        ${OPENSSL_INCLUDE_DIR}
        ${PAHO_INCLUDE_DIRS}
        ${EXPAT_INCLUDE_DIRS}
        ${DLFCN_INCLUDES}
        ${LOCAL_WIN_INCLUDE}
        ${LOCAL_SYSLOG_INCLUDE} )
 endif()   

if (WIN32)

    target_link_libraries(vscp-works-qt PRIVATE     
        Qt5::Widgets
        Qt5::Core 
        Qt5::Network
        Qt5::Quick 
        Qt5::Sql
        Qt5::Charts
        PThreads4W::PThreads4W_CXXEXC
        OpenSSL::SSL 
        OpenSSL::Crypto
        OpenSSL::SSL OpenSSL::Crypto
        eclipse-paho-mqtt-c::paho-mqtt3a eclipse-paho-mqtt-c::paho-mqtt3c eclipse-paho-mqtt-c::paho-mqtt3as eclipse-paho-mqtt-c::paho-mqtt3cs
        expat::expat
        dlfcn-win32::dl)
    else()
    target_link_libraries(vscp-works-qt PRIVATE     
        ${Qt5Widgets_LIBRARIES}
        ${Qt5Core_LIBRARIES}
        Qt5::Network
        Qt5::Quick 
        Qt5::Sql
        Qt5::Charts
        m
        dl
        Threads::Threads
        OpenSSL::SSL 
        OpenSSL::Crypto
        ${OPENSSL_LIBRARIES}
        ${PAHO_LIBRARIES}
        ${EXPAT_LIBRARIES}
        ${DLFCN_LIBS})
endif()

# https://www3.sra.co.jp/qt/relation/doc/qtdoc/cmake-manual.html
qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
qt5_add_resources(src/vscpworks.qrc) 


if(WIN32)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DWIN32_LEAN_AND_MEANS)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
    set(LIBS_SYSTEM ws2_32)
elseif(UNIX)
    set(LIBS_SYSTEM c stdc++)
endif()

install(TARGETS vscp-works-qt
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES ${CMAKE_SOURCE_DIR}/install/vscp-works-qt.desktop
        DESTINATION ${INSTALL_PATH}/share/applications/) 

# icons
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/icons/ DESTINATION ${INSTALL_PATH}/share/icons/) 

# Event database etc
install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/install/share/ DESTINATION ${INSTALL_PATH}/share/vscp-works-qt/)
