# VSCPConfig.cmake.in
#
# CMake package configuration template for the VSCP common library.
#
# After installation this generates VSCPConfig.cmake which enables
# consumers to use find_package(VSCP) in their CMakeLists.txt.
#
# Copyright (C) 2000-2026 Ake Hedman, the VSCP project
# SPDX-License-Identifier: MIT
#
# Usage in consuming project:
#   find_package(VSCP REQUIRED)
#   target_link_libraries(myapp PRIVATE VSCP::vscp_common)


####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was VSCPConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

include(CMakeFindDependencyMacro)

# Required dependencies that consumers must also find
find_dependency(Threads REQUIRED)
find_dependency(OpenSSL REQUIRED)
find_dependency(EXPAT REQUIRED)
find_dependency(CURL REQUIRED)

# Add the custom Find modules shipped with VSCP so consumers can find Mosquitto
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/modules")
find_dependency(Mosquitto REQUIRED)

# Import the exported targets
include("${CMAKE_CURRENT_LIST_DIR}/VSCPTargets.cmake")

check_required_components(vscp_common vscp_util)
