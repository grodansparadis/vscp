cmake_minimum_required(VERSION 3.5)
project(man LANGUAGES CXX C)
set(PACKAGE_AUTHOR "Ake Hedman, the VSCP Project")

# --- System Libraries ---
include(GNUInstallDirs)

set(PANDOCCOMMAND_FOUND 0)
find_program(PANDOCCOMMAND_PATH
  NAMES pandoc
  DOC "Path to Pandoc processor. Used to generate man pages from Markdown."
)

if(NOT EXISTS ${PANDOCCOMMAND_PATH})
  message(WARNING "Pandoc processor not found, man pages will not be generated. Install pandoc or use the CMAKE_PROGRAM_PATH variable.")
else()
  set(PANDOCCOMMAND_FOUND 1)
endif()

# if (NOT ${PANDOCCOMMAND_FOUND})
#     message("* * * PanDoc was not found * * *")
# endif()

# man pages
if (${PANDOCCOMMAND_FOUND})
    ADD_CUSTOM_TARGET(man ALL)

    ADD_CUSTOM_COMMAND(
        TARGET man
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/vscpd.8.md
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND pandoc -s -t man ${CMAKE_CURRENT_SOURCE_DIR}/vscpd.8.md -o ${CMAKE_CURRENT_BINARY_DIR}/vscpd.8
        OUTPUTS ${CMAKE_CURRENT_BINARY_DIR}/vscpd.8
)
endif()

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/vscpd.8
  DESTINATION ${CMAKE_INSTALL_MANDIR}/man8
)