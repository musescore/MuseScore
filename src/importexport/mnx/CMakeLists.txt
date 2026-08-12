# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# .mnx Import Module (Experimental)

muse_create_module(iex_mnx)

include(FetchContent)

set(mnxdom_BUILD_TESTING OFF CACHE BOOL "Do not build tests for mnx")
if(MSS_USE_SYSTEM_MNXDOM)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(mnxdom REQUIRED IMPORTED_TARGET mnxdom)
    set(MNXDOM_LIB PkgConfig::mnxdom)
    target_compile_definitions(iex_mnx PRIVATE MNXDOM_SYSTEM)
    # The fetched path derives this from the pinned tag. Nothing sets it here, so record the
    # package version instead of leaving provenance empty or carrying a stale cached value.
    set(MNXDOM_GIT_COMMIT "system-${mnxdom_VERSION}" CACHE STRING
        "mnxdom source control revision to embed in provenance" FORCE)
    message(STATUS "Using system-installed mnxdom ${mnxdom_VERSION}")
else()
    # nlohmann_json and json_schema_validator come from muse_deps (see
    # buildscripts/cmake/DependencyManifest.cmake); mnxdom finds them there
    # instead of fetching and building its own copies. The w3c schema and
    # examples are vendored in mnxdom itself, so nothing to point it at.
    get_property(NLOHMANN_JSON_PREFIX GLOBAL PROPERTY nlohmann_json_PREFIX)
    get_property(JSON_SCHEMA_VALIDATOR_PREFIX GLOBAL PROPERTY json_schema_validator_PREFIX)
    if(NOT NLOHMANN_JSON_PREFIX OR NOT JSON_SCHEMA_VALIDATOR_PREFIX)
        # Without this, mnxdom would quietly fetch and build its own copies
        message(FATAL_ERROR "[mnxdom] nlohmann_json / json_schema_validator not resolved; "
                            "DependencyManifest.cmake must run before this module")
    endif()
    list(PREPEND CMAKE_PREFIX_PATH "${NLOHMANN_JSON_PREFIX}" "${JSON_SCHEMA_VALIDATOR_PREFIX}")
    set(USE_SYSTEM_NLOHMANN_JSON ON CACHE BOOL "" FORCE)
    set(USE_SYSTEM_JSON_SCHEMA_VALIDATOR ON CACHE BOOL "" FORCE)
    message(STATUS "Using muse_deps nlohmann_json from ${NLOHMANN_JSON_PREFIX}")
    message(STATUS "Using muse_deps json_schema_validator from ${JSON_SCHEMA_VALIDATOR_PREFIX}")

    set(MNXDOM_GIT_TAG 45bf46221c00411e968c851ff9a1be4e202af5c2)
    string(SUBSTRING "${MNXDOM_GIT_TAG}" 0 7 MNXDOM_GIT_COMMIT)
    FetchContent_Declare(
        mnxdom
        GIT_REPOSITORY https://github.com/rpatters1/mnxdom.git
        GIT_TAG        ${MNXDOM_GIT_TAG}
    )
    set(MNXDOM_GIT_COMMIT "${MNXDOM_GIT_COMMIT}" CACHE STRING "mnxdom source control revision to embed in provenance" FORCE)
    FetchContent_MakeAvailable(mnxdom)
    set(MNXDOM_LIB mnxdom)
    message(STATUS "Using fetched mnxdom")
endif()

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
    ${MNXDOM_LIB}
    engraving
)

if (MSVC)
    target_compile_options(iex_mnx PRIVATE /Zc:preprocessor)
    set_target_properties(iex_mnx PROPERTIES DISABLE_PRECOMPILE_HEADERS ON)
endif()

if (MUE_BUILD_IMPORTEXPORT_TESTS)
    add_subdirectory(tests)
endif()
