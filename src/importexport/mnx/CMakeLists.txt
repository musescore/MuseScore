# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# .mnx Import Module (Experimental)

muse_create_module(iex_mnx)

include(FetchContent)

set(mnxdom_BUILD_TESTING OFF CACHE BOOL "Do not build tests for mnx")
FetchContent_Declare(
    mnxdom
    GIT_REPOSITORY https://github.com/rpatters1/mnxdom.git
    GIT_TAG        main
)

FetchContent_MakeAvailable(mnxdom)

target_sources(iex_mnx PRIVATE
    mnxmodule.cpp
    mnxmodule.h
    internal/export/mnxexporter.cpp
    internal/export/mnxexporter.h
    internal/import/mnximporter.cpp
    internal/import/mnximporter.h
    internal/import/mnximportmarkings.cpp
    internal/import/mnximportpartmeasures.cpp
    internal/shared/mnxtypesconv.cpp
    internal/shared/mnxtypesconv.h
    internal/notationmnxreader.cpp
    internal/notationmnxreader.h
    internal/notationmnxwriter.cpp
    internal/notationmnxwriter.h
)

target_link_libraries(iex_mnx PRIVATE
    mnxdom            # Link MnxDom library
    engraving         # MuseScore expects importers to link to "engraving" usually
)
