cmake_minimum_required(VERSION 3.10)

# set the project name
project(test_vscpremotetcpif LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# include(FetchContent)
# FetchContent_Declare(
#   googletest
#   URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
# )
# # For Windows: Prevent overriding the parent project's compiler/linker settings
# set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(googletest)

# set the module path
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

message(STATUS "Build dir: " ${PROJECT_SOURCE_DIR})

find_package(GTest CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# pthreads
set (THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

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


if (WIN32)    
  find_package(pthreads REQUIRED)
  find_path(PTHREAD_INCLUDE_DIR pthread.h)

  target_include_directories(test_vscpremotetcpif PUBLIC 
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/
    ${PROJECT_SOURCE_DIR}/../../src/common/
    ${PROJECT_SOURCE_DIR}/../../src/common/third_party
    ${PROJECT_SOURCE_DIR}/../../src/common/third_party/nlohmann
    ${PROJECT_SOURCE_DIR}/../../src/common/third_party/spdlog-1.9.2/include/
    ${OPENSSL_INCLUDE_DIR}
    ${EXPAT_INCLUDE_DIRS}
  )
else()
  target_include_directories(test_vscpremotetcpif PUBLIC 
    ${PROJECT_SOURCE_DIR}/../../src/vscp/common/
    ${PROJECT_SOURCE_DIR}/../../src/common/
    ${PROJECT_SOURCE_DIR}/../../src/common/third_party
    ${PROJECT_SOURCE_DIR}/../../src/common/third_party/nlohmann
    ${PROJECT_SOURCE_DIR}/../../src/common/third_party/spdlog-1.9.2/include/
    ${OPENSSL_INCLUDE_DIR}
    ${EXPAT_INCLUDE_DIRS}
  )
endif()  
    
if (WIN32)
  target_link_libraries(test_vscpremotetcpif PRIVATE
    PThreads4W::PThreads4W_CXXEXC
    OpenSSL::SSL OpenSSL::Crypto
    ${OPENSSL_LIBRARIES}
    ${EXPAT_LIBRARIES}
    GTest::gtest
    GTest::gmock
  )
else()
  target_link_libraries(test_vscpremotetcpif PRIVATE
    pthread
    OpenSSL::SSL OpenSSL::Crypto
    ${OPENSSL_LIBRARIES}
    ${EXPAT_LIBRARIES}
    GTest::gtest
    GTest::gmock
  )    
endif()


