cmake_minimum_required(VERSION 3.10)

# set the project name
project(test LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

message(STATUS "Build dir: " ${PROJECT_SOURCE_DIR})

find_package(OpenSSL REQUIRED)

# TODO Check SLL version and set variable accordingly
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DOPENSSL_API_1_1")
set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DOPENSSL_API_1_1")

include(FindEXPAT)
find_package(EXPAT REQUIRED)
include_directories(${EXPAT_INCLUDE_DIRS})

# add the executable
add_executable(test 
    test-vscp-client-tcp.cpp
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/guid.h
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/guid.cpp
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscphelper.h
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscpdatetime.cpp
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscpdatetime.h
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscphelper.cpp
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscpremotetcpif.h
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscpremotetcpif.cpp
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscp_client_base.h
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscp_client_base.cpp
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscp_client_tcp.h
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscp_client_tcp.cpp
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
    ${PROJECT_SOURCE_DIR}/../../third_party/fastpbkdf2/fastpbkdf2.h
    ${PROJECT_SOURCE_DIR}/../../third_party/fastpbkdf2/fastpbkdf2.c
    ${PROJECT_SOURCE_DIR}/../../third_party/nlohmann/include/nlohmann/json.hpp
    ${PROJECT_SOURCE_DIR}/../../third_party/spdlog/include/spdlog/spdlog.h
    )

target_include_directories(test PUBLIC 
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/
    ${PROJECT_SOURCE_DIR}/../../src/common/
    ${PROJECT_SOURCE_DIR}/../../third_party/fastpbkdf2/
    ${PROJECT_SOURCE_DIR}/../../third_party/nlohmann/include/nlohmann
    ${PROJECT_SOURCE_DIR}/../../third_party/spdlog/include/

    ${OPENSSL_INCLUDE_DIR}
    ${EXPAT_INCLUDE_DIRS}
    )

target_link_libraries(test PRIVATE m
    dl
    pthread
    OpenSSL::SSL OpenSSL::Crypto
    ${OPENSSL_LIBRARIES}
    ${EXPAT_LIBRARIES}
    )    

#add_library(MathFunctions mysqrt.cxx)
