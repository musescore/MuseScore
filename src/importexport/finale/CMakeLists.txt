# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Musx Import Module (Experimental)

declare_module(iex_finale)

include(FetchContent)
include(GetCompilerInfo)

# temporary for reference and debugging
set(MUSX_DISPLAY_NODE_NAMES ON CACHE BOOL "Log node names to trace as they are processed" FORCE)

set(musx_BUILD_TESTING OFF CACHE BOOL "Do not build tests for musx")
FetchContent_Declare(
    musx
    GIT_REPOSITORY https://github.com/rpatters1/musxdom.git
    GIT_TAG        dev/musescore-experiment
)

FetchContent_MakeAvailable(musx)

if (CC_IS_MSVC)
    include(FindZlibStatic)
    set(Z_INCLUDE ${DEPENDENCIES_INC}/zlib)
    list(APPEND MODULE_INCLUDE_PRIVATE ${Z_INCLUDE})
endif ()

set(MODULE_SRC
    ${TINYXML_MODULE_SRC}
    ${CMAKE_CURRENT_LIST_DIR}/third_party/musx/TinyXmlImpl.h
    ${CMAKE_CURRENT_LIST_DIR}/third_party/score_encoder.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationfinalereader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationfinalereader.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/finaletypesconv.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/finaletypesconv.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/importenigmaxml.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/importenigmaxml.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/importfinaleentries.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/importfinalelogger.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/importfinalelogger.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/importfinalescoremap.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/importfinalescoremap.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/importfinalestyles.cpp
    ${CMAKE_CURRENT_LIST_DIR}/finalemodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/finalemodule.h
)

set(MODULE_LINK
    musx              # Link your MusxDom library
    engraving         # MuseScore expects importers to link to "engraving" usually
)

setup_module()

if (MUE_BUILD_IMPORTEXPORT_TESTS)
    add_subdirectory(tests)
endif()
