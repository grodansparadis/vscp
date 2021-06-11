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
add_executable(unittest 
    unittest.cpp
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/guid.h
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/guid.cpp
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscphelper.h
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscphelper.cpp
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscpdatetime.cpp
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscpdatetime.h 
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/clientlist.cpp
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/clientlist.h    
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/vscp.h
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

target_include_directories(unittest PUBLIC 
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/
    ${PROJECT_SOURCE_DIR}/../../src/common/
    ${PROJECT_SOURCE_DIR}/../../src/common/third_party
    ${PROJECT_SOURCE_DIR}/../../src/common/third_party/nlohmann
    ${PROJECT_SOURCE_DIR}/../../src/common/third_party/spdlog-1.8.5/include
    ${PROJECT_SOURCE_DIR}/../../googletest/googletest/include
    ${OPENSSL_INCLUDE_DIR}
    ${EXPAT_INCLUDE_DIRS}
)

target_link_libraries(unittest PRIVATE m
    dl
    pthread
    OpenSSL::SSL OpenSSL::Crypto
    ${OPENSSL_LIBRARIES}
    ${EXPAT_LIBRARIES}
    gtest
    gtest_main
)    

#add_library(MathFunctions mysqrt.cxx)
