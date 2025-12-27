# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# .musx Import Module (Experimental)

muse_create_module(iex_finale)

include(FetchContent)
include(GetCompilerInfo)

# temporary for reference and debugging
set(MUSX_DISPLAY_NODE_NAMES ON CACHE BOOL "Log node names to trace as they are processed" FORCE)

set(musx_BUILD_TESTING OFF CACHE BOOL "Do not build tests for musx")
FetchContent_Declare(
    musx
    URL https://github.com/rpatters1/musxdom/archive/refs/tags/v3.0.0.tar.gz
    URL_HASH SHA256=46605a98c56894175a49a7f3db5aa390246e159d554e73dc6be7c312b044f3f3
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(musx)

set(smufl_mapping_BUILD_TESTING OFF CACHE BOOL "Do not build tests for musx")
FetchContent_Declare(
    smufl_mapping
    URL https://github.com/rpatters1/smufl-mapping/archive/refs/tags/v1.1.0.tar.gz
    URL_HASH SHA256=e921465b9a99920ce8fc2e14126c1fe1840a710be60e0f24130c8df75d0d7a85
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(smufl_mapping)

if (CC_IS_MSVC)
    include(FindZlibStatic)
    target_link_libraries(iex_finale PRIVATE zlibstat)
    target_include_directories(iex_finale PRIVATE ${DEPENDENCIES_INC}/zlib)
endif ()

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
    engraving         # MuseScore expects importers to link to "engraving" usually
)

if (MUE_BUILD_IMPORTEXPORT_TESTS)
    add_subdirectory(tests)
endif()
