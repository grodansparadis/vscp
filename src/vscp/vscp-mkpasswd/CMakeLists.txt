cmake_minimum_required(VERSION 3.5)

# set the project name
project(vscp-mkpasswd LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

include(GNUInstallDirs)

# Disable abi warnings (nlohmann)
SET(GCC_COVERAGE_COMPILE_FLAGS "-Wno-psabi -DOPENSSL_API_1_1")
#SET(GCC_COVERAGE_LINK_FLAGS    "-lm")

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

message(STATUS "Build dir: " ${PROJECT_SOURCE_DIR})

find_package(OpenSSL REQUIRED)

# TODO Check SLL version and set variable accordingly
#set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -DOPENSSL_API_1_1")
#set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DOPENSSL_API_1_1")

# Disable abi warnings (nlohmann)
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
#SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

include(FindEXPAT)
find_package(EXPAT REQUIRED)
include_directories(${EXPAT_INCLUDE_DIRS})

# add the executable
add_executable(vscp-mkpasswd 
    vscp-mkpasswd.cpp
    ${PROJECT_SOURCE_DIR}/../../../src/vscp/common/guid.h
    ${PROJECT_SOURCE_DIR}/../../../src/vscp/common/guid.cpp
    ${PROJECT_SOURCE_DIR}/../../../src/vscp/common/vscphelper.h
    ${PROJECT_SOURCE_DIR}/../../../src/vscp/common/vscphelper.cpp
    ${PROJECT_SOURCE_DIR}/../../../src/vscp/common/vscpdatetime.cpp
    ${PROJECT_SOURCE_DIR}/../../../src/vscp/common/vscpdatetime.h 
    ${PROJECT_SOURCE_DIR}/../../../src/vscp/common/userlist.cpp
    ${PROJECT_SOURCE_DIR}/../../../src/vscp/common/userlist.h    
    ${PROJECT_SOURCE_DIR}/../../../src/vscp/common/vscp.h
    ${PROJECT_SOURCE_DIR}/../../../src/common/vscpbase64.h
    ${PROJECT_SOURCE_DIR}/../../../src/common/vscpbase64.c
    ${PROJECT_SOURCE_DIR}/../../../src/common/vscpmd5.h
    ${PROJECT_SOURCE_DIR}/../../../src/common/vscpmd5.c
    ${PROJECT_SOURCE_DIR}/../../../src/common/crc.h
    ${PROJECT_SOURCE_DIR}/../../../src/common/crc.c
    ${PROJECT_SOURCE_DIR}/../../../src/common/crc8.h
    ${PROJECT_SOURCE_DIR}/../../../src/common/crc8.c
    ${PROJECT_SOURCE_DIR}/../../../src/common/vscp-aes.h
    ${PROJECT_SOURCE_DIR}/../../../src/common/vscp-aes.c
    ${PROJECT_SOURCE_DIR}/../../../third_party/fastpbkdf2/fastpbkdf2.h
    ${PROJECT_SOURCE_DIR}/../../../third_party/fastpbkdf2/fastpbkdf2.c
    ${PROJECT_SOURCE_DIR}/../../../third_party/nlohmann/include/nlohmann/json.hpp
    )

target_include_directories(vscp-mkpasswd PUBLIC 
    ${PROJECT_SOURCE_DIR}/../../../src/vscp/common/
    ${PROJECT_SOURCE_DIR}/../../../src/common/
    ${PROJECT_SOURCE_DIR}/../../../src/common/third_party
    ${PROJECT_SOURCE_DIR}/../../../third_party/nlohmann/include
    ${PROJECT_SOURCE_DIR}/../../../third_party/spdlog/include
    ${PROJECT_SOURCE_DIR}/../../../third_party/fastpbkdf2/
    ${PROJECT_SOURCE_DIR}/../../../third_party/mustache/
    ${PROJECT_SOURCE_DIR}/../../../googletest/googletest/include
    ${OPENSSL_INCLUDE_DIR}
    ${EXPAT_INCLUDE_DIRS}
)

target_link_libraries(vscp-mkpasswd PRIVATE m
    dl
    pthread
    OpenSSL::SSL OpenSSL::Crypto
    ${OPENSSL_LIBRARIES}
    ${EXPAT_LIBRARIES}
#    gtest
#    gtest_main
)    

install(TARGETS vscp-mkpasswd
	RUNTIME DESTINATION
       	${CMAKE_INSTALL_BINDIR})

#add_library(MathFunctions mysqrt.cxx)
