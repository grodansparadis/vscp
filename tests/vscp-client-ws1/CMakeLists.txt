cmake_minimum_required(VERSION 3.10)

# set the project name
project(test LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set the module path
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

message(STATUS "Build dir: " ${PROJECT_SOURCE_DIR})

find_package(OpenSSL REQUIRED)

# TODO Check SLL version and set variable accordingly
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DOPENSSL_API_1_1")
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DOPENSSL_API_1_1")

set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DOPENSSL_API_1_1")

include(FindEXPAT)
find_package(EXPAT REQUIRED)
include_directories(${EXPAT_INCLUDE_DIRS})

include(FindLibWebSockets)

set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DUSE_WEBSOCKET")

# add the executable
add_executable(test 
    test-vscp-client-ws1.cpp
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/guid.h
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/guid.cpp
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscphelper.h
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscpdatetime.cpp
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscpdatetime.h
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscphelper.cpp
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscpcanaldeviceif.h
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscpcanaldeviceif.cpp
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscp_client_base.h
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscp_client_base.cpp
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscp_client_ws1.h
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscp_client_ws1.cpp
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscp.h
    ${PROJECT_SOURCE_DIR}/../../src/common/vscpbase64.h
    ${PROJECT_SOURCE_DIR}/../../src/common/vscpbase64.c
    ${PROJECT_SOURCE_DIR}/../../src/common/vscpmd5.h
    ${PROJECT_SOURCE_DIR}/../../src/common/vscpmd5.c
    ${PROJECT_SOURCE_DIR}/../../src/common/crc.h
    ${PROJECT_SOURCE_DIR}/../../src/common/crc.c
    ${PROJECT_SOURCE_DIR}/../../src/common/crc8.h
    ${PROJECT_SOURCE_DIR}/../../src/common/crc8.c
    ${PROJECT_SOURCE_DIR}/../../src/common/vscp-aes.h
    ${PROJECT_SOURCE_DIR}/../../src/common/vscp-aes.c
    ${PROJECT_SOURCE_DIR}/../../src/common/fastpbkdf2.h
    ${PROJECT_SOURCE_DIR}/../../src/common/fastpbkdf2.c
    ${PROJECT_SOURCE_DIR}/../../src/common/third_party/civetweb-1.13/include/civetweb.h
    ${PROJECT_SOURCE_DIR}/../../src/common/third_party/civetweb-1.13/src/civetweb.c
    ${PROJECT_SOURCE_DIR}/../../src/common/third_party/nlohmann/json.hpp
    )

target_include_directories(test PUBLIC 
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/
    ${PROJECT_SOURCE_DIR}/../../src/common/
    ${PROJECT_SOURCE_DIR}/../../src/common/third_party/civetweb-1.13/include
    ${PROJECT_SOURCE_DIR}/../../src/common/third_party/nlohmann/
    ${OPENSSL_INCLUDE_DIR}
    ${EXPAT_INCLUDE_DIRS}
    ${MOSQUITTO_INCLUDE_DIR}
    )

target_link_libraries(test PRIVATE m
    dl
    pthread
    OpenSSL::SSL OpenSSL::Crypto
    ${OPENSSL_LIBRARIES}
    ${EXPAT_LIBRARIES}
    )    

#add_library(MathFunctions mysqrt.cxx)
