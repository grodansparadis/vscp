cmake_minimum_required(VERSION 3.10)

# set the project name
project(test_vscp_client_level2 LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(GTest REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# A fake Level II driver that the client loads for real
add_library(vscpl2drv-fake MODULE fakedriver.cpp)
target_include_directories(vscpl2drv-fake PRIVATE
    $<TARGET_PROPERTY:vscp_core,INTERFACE_INCLUDE_DIRECTORIES>)
target_link_libraries(vscpl2drv-fake PRIVATE Threads::Threads)

# A shared library that is not a Level II driver
add_library(vscpl2drv-notadriver MODULE notadriver.cpp)

add_executable(unittest_vscp_client_level2 unittest.cpp)
add_dependencies(unittest_vscp_client_level2 vscpl2drv-fake vscpl2drv-notadriver)
target_compile_definitions(unittest_vscp_client_level2 PRIVATE
    FAKE_DRIVER_PATH="$<TARGET_FILE:vscpl2drv-fake>"
    NOT_A_DRIVER_PATH="$<TARGET_FILE:vscpl2drv-notadriver>")

target_link_libraries(unittest_vscp_client_level2 PRIVATE
    vscp_common
    ${CMAKE_DL_LIBS}
    Threads::Threads
    GTest::GTest
    GTest::Main
)
