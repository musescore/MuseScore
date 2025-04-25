# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Musx Import Module (Experimental)

declare_module(iex_musx)

include(FetchContent)

set(MUSX_USE_QTXML ON CACHE BOOL "Enable Qt xml parsing classes" FORCE)
set(musx_BUILD_TESTING OFF CACHE BOOL "Do not build tests for musx")
FetchContent_Declare(
    musx
    GIT_REPOSITORY https://github.com/rpatters1/musxdom.git
    GIT_TAG        dev/musescore-experiment
)

FetchContent_MakeAvailable(musx)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationmusxreader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationmusxreader.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/musx_import.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/musx_import.h
    ${CMAKE_CURRENT_LIST_DIR}/musxmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/musxmodule.h
    )

set(MODULE_LINK
    musx              # Link your MusxDom library
    engraving         # MuseScore expects importers to link to "engraving" usually
    )

setup_module()
