include(${CMAKE_CURRENT_LIST_DIR}/VSCPDependencies.cmake)

if(NOT TARGET vscp_mongoose)
    find_package(OpenSSL REQUIRED)
    add_library(vscp_mongoose SHARED "${VSCP_MONGOOSE_SOURCE_DIR}/mongoose.c")
    target_include_directories(vscp_mongoose PUBLIC
        $<BUILD_INTERFACE:${VSCP_MONGOOSE_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include/vscp/third-party/mongoose>)
    target_compile_definitions(vscp_mongoose PUBLIC MG_TLS=MG_TLS_OPENSSL)
    target_link_libraries(vscp_mongoose PUBLIC OpenSSL::SSL OpenSSL::Crypto)
    if(WIN32)
        set_target_properties(vscp_mongoose PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
        target_link_libraries(vscp_mongoose PUBLIC ws2_32)
    endif()
    if(VSCP_INSTALL)
        install(FILES "${VSCP_MONGOOSE_SOURCE_DIR}/mongoose.h"
            DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vscp/third-party/mongoose)
    endif()
endif()
