cmake_minimum_required(VERSION 3.10)

# set the project name
project(test_vscpremotetcpif LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set the module path
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

message(STATUS "Build dir: " ${PROJECT_SOURCE_DIR})

find_package(GTest CONFIG REQUIRED)

find_package(OpenSSL REQUIRED)

# TODO Check SLL version and set variable accordingly
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DOPENSSL_API_1_1")
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DOPENSSL_API_1_1")

include(FindEXPAT)
find_package(EXPAT REQUIRED)
include_directories(${EXPAT_INCLUDE_DIRS})

# add the executable
add_executable(test_vscpremotetcpif 
    ${PROJECT_SOURCE_DIR}/test-vscpremotetcpif.cpp
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscpremotetcpif.h
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscpremotetcpif.cpp
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/guid.h
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/guid.cpp
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscphelper.h
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscpdatetime.cpp
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscpdatetime.h
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscphelper.cpp
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscpcanaldeviceif.h
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscpcanaldeviceif.cpp
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscp.h
    ${PROJECT_SOURCE_DIR}/../../src/common/sockettcp.h
    ${PROJECT_SOURCE_DIR}/../../src/common/sockettcp.c
    ${PROJECT_SOURCE_DIR}/../../src/common/vscpbase64.h
    ${PROJECT_SOURCE_DIR}/../../src/common/vscpbase64.c
    ${PROJECT_SOURCE_DIR}/../../src/common/vscpmd5.h
    ${PROJECT_SOURCE_DIR}/../../src/common/vscpmd5.c
    ${PROJECT_SOURCE_DIR}/../../src/common/crc.h
    ${PROJECT_SOURCE_DIR}/../../src/common/crc.c
    ${PROJECT_SOURCE_DIR}/../../src/common/crc8.h
    ${PROJECT_SOURCE_DIR}/../../src/common/crc8.c
    ${PROJECT_SOURCE_DIR}/../../src/common/vscp_aes.h
    ${PROJECT_SOURCE_DIR}/../../src/common/vscp_aes.c
    ${PROJECT_SOURCE_DIR}/../../src/common/fastpbkdf2.h
    ${PROJECT_SOURCE_DIR}/../../src/common/fastpbkdf2.c
    ${PROJECT_SOURCE_DIR}/../../src/common/third_party/nlohmann/json.hpp
    )

target_include_directories(test_vscpremotetcpif PUBLIC 
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/
    ${PROJECT_SOURCE_DIR}/../../src/common/
    ${PROJECT_SOURCE_DIR}/../../src/common/third_party
    ${PROJECT_SOURCE_DIR}/../../src/common/third_party/nlohmann
    ${PROJECT_SOURCE_DIR}/../../src/common/third_party/spdlog-1.8.5/include/
    ${OPENSSL_INCLUDE_DIR}
    ${EXPAT_INCLUDE_DIRS}
    ${MOSQUITTO_INCLUDE_DIR}
    )

target_link_libraries(test_vscpremotetcpif PRIVATE m
  dl
  pthread
  OpenSSL::SSL OpenSSL::Crypto
  ${OPENSSL_LIBRARIES}
  ${EXPAT_LIBRARIES}
  GTest::gtest
  GTest::gmock
)    

