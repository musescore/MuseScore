# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# .musx Import Module (Experimental)

muse_create_module(iex_finale)

include(FetchContent)
find_package(ZLIB REQUIRED)

# turn on for reference and debugging as needed
set(MUSX_DISPLAY_NODE_NAMES OFF CACHE BOOL "Log node names to trace as they are processed" FORCE)

set(musx_BUILD_TESTING OFF CACHE BOOL "Do not build tests for musx")
FetchContent_Declare(
    musx
    GIT_REPOSITORY https://github.com/rpatters1/musxdom.git
    GIT_TAG        main
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(musx)

set(smufl_mapping_BUILD_TESTING OFF CACHE BOOL "Do not build tests for musx")
FetchContent_Declare(
    smufl_mapping
    GIT_REPOSITORY https://github.com/rpatters1/smufl-mapping.git
    GIT_TAG        main
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(smufl_mapping)

target_sources(iex_finale PRIVATE
    third_party/score_encoder.h
    internal/notationfinalereader.cpp
    internal/notationfinalereader.h
    internal/finaleconfiguration.cpp
    internal/finaleconfiguration.h
    internal/finaletypesconv.cpp
    internal/finaletypesconv.h
    internal/musxxmldom.h
    internal/importfinale.cpp
    internal/importfinale.h
    internal/importfinaleentries.cpp
    internal/importfinalelogger.cpp
    internal/importfinalelogger.h
    internal/importfinalescoremap.cpp
    internal/importfinaleparser.cpp
    internal/importfinaleparser.h
    internal/importfinalestyles.cpp
    internal/importfinalesmartshapes.cpp
    internal/importfinaletext.cpp
    internal/importfinalearticulations.cpp
    internal/text/finaletextconv.cpp
    internal/text/finaletextconv.h
    ifinaleconfiguration.h
    finalemodule.cpp
    finalemodule.h
)

target_link_libraries(iex_finale PRIVATE
    musx              # Link musxdom library
    smufl_mapping     # Link smufl_mapping library
    ZLIB::ZLIB        # Link zlib for direct gunzip support
    engraving         # MuseScore expects importers to link to "engraving" usually
)

if (MUE_BUILD_IMPORTEXPORT_TESTS)
    add_subdirectory(tests)
endif()
