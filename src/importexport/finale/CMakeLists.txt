# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# .musx Import Module (Experimental)

declare_module(iex_finale)

include(FetchContent)
include(GetCompilerInfo)

# temporary for reference and debugging
set(MUSX_DISPLAY_NODE_NAMES ON CACHE BOOL "Log node names to trace as they are processed" FORCE)

set(musx_BUILD_TESTING OFF CACHE BOOL "Do not build tests for musx")
FetchContent_Declare(
    musx
    GIT_REPOSITORY https://github.com/rpatters1/musxdom.git
    GIT_TAG        dev/voiced-parts
    #URL https://github.com/rpatters1/musxdom/archive/refs/tags/v2.0.0.tar.gz
    #URL_HASH SHA256=f976d65257f391bf6ceb0b6a1a3050deedb63a206c2bf2d193b5a0a581f5b3b4
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(musx)

set(smufl_mapping_BUILD_TESTING OFF CACHE BOOL "Do not build tests for musx")
FetchContent_Declare(
    smufl_mapping
    GIT_REPOSITORY https://github.com/rpatters1/smufl-mapping.git
    GIT_TAG        main
    #URL https://github.com/rpatters1/smufl-mapping/archive/refs/tags/v1.0.0.tar.gz
    #URL_HASH SHA256=53db1524ecfae1e8d3022a796b05bf4dc85ebbcd522c88c431e3db221263d3ab
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(smufl_mapping)

if (CC_IS_MSVC)
    include(FindZlibStatic)
    set(Z_INCLUDE ${DEPENDENCIES_INC}/zlib)
    list(APPEND MODULE_INCLUDE_PRIVATE ${Z_INCLUDE})
endif ()

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/third_party/score_encoder.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationfinalereader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationfinalereader.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/finaletypesconv.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/finaletypesconv.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/musxxmldom.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/importfinale.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/importfinale.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/importfinaleentries.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/importfinalelogger.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/importfinalelogger.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/importfinalescoremap.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/importfinaleparser.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/importfinaleparser.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/importfinalestyles.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/importfinalesmartshapes.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/importfinaletext.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/importfinalearticulations.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/text/finaletextconv.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/text/finaletextconv.h
    ${CMAKE_CURRENT_LIST_DIR}/finalemodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/finalemodule.h
)

set(MODULE_LINK
    musx              # Link musxdom library
    smufl_mapping     # Link smufl_mapping library
    engraving         # MuseScore expects importers to link to "engraving" usually
)

setup_module()

if (MUE_BUILD_IMPORTEXPORT_TESTS)
    add_subdirectory(tests)
endif()
