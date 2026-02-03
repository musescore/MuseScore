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
    GIT_TAG        bb5661363303de0bcabc2ae45325f9f66b2ff190
)

FetchContent_MakeAvailable(mnxdom)

target_sources(iex_mnx PRIVATE
    mnxmodule.cpp
    mnxmodule.h
    imnxconfiguration.h
    internal/export/mnxexportglobal.cpp
    internal/export/mnxexportlayouts.cpp
    internal/export/mnxexportparts.cpp
    internal/export/mnxexportsequences.cpp
    internal/export/mnxexporter.cpp
    internal/export/mnxexporter.h
    internal/mnxconfiguration.cpp
    internal/mnxconfiguration.h
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
    mnxdom
    engraving
)

if (MSVC)
    target_compile_options(iex_mnx PRIVATE /Zc:preprocessor)
    set_target_properties(iex_mnx PROPERTIES DISABLE_PRECOMPILE_HEADERS ON)
endif()

if (MUE_BUILD_IMPORTEXPORT_TESTS)
    add_subdirectory(tests)
endif()
