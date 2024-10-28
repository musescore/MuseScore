# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2024 MuseScore Limited
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

## Declare
# declare_module(somename) - set module (target) name

## Setup
# set(MODULE somename)                        - set module (target) name
# set(MODULE_ALIAS somename)                  - set module (target) alias name
# set(MODULE_ROOT ${CMAKE_CURRENT_LIST_DIR})  - set module root
# set(MODULE_INCLUDE ...)                     - set include (by default see below include_directories)
# set(MODULE_INCLUDE_PRIVATE ...)             - set private include
# set(MODULE_DEF ...)                         - set definitions
# set(MODULE_SRC ...)                         - set sources and headers files
# set(MODULE_LINK ...)                        - set libraries for link
# set(MODULE_LINK_PUBLIC ...)                 - set libraries for link and transitive link
# set(MODULE_LINK_GLOBAL ON/OFF)              - set whether to link with `global` module (default ON)
# set(MODULE_QRC somename.qrc)                - set resource (qrc) file
# set(MODULE_BIG_QRC somename.qrc)            - set big resource (qrc) file
# set(MODULE_QML_IMPORT ...)                  - set Qml import for QtCreator (so that there is code highlighting, jump, etc.)
# set(MODULE_QMLEXT_IMPORT ...)               - set Qml extensions import for QtCreator (so that there is code highlighting, jump, etc.)
# set(MODULE_USE_PCH ON/OFF)                  - set whether to use precompiled headers for this module (default ON)
# set(MODULE_USE_UNITY ON/OFF)                - set whether to use unity build for this module (default ON)
# set(MODULE_USE_COVERAGE ON)                 - set whether to use coverage for this module (default ON)
# set(MODULE_IS_STUB ON)                      - set a mark that the module is stub

# After all the settings you need to do:
# setup_module()

macro(declare_module name)
    set(MODULE ${name})
    # just reset all settings
    unset(MODULE_ALIAS)
    unset(MODULE_ROOT)
    unset(MODULE_INCLUDE)
    unset(MODULE_DEF)
    unset(MODULE_SRC)
    unset(MODULE_LINK)
    unset(MODULE_LINK_PUBLIC)
    set(MODULE_LINK_GLOBAL ON)
    set(MODULE_USE_QT ON)
    unset(MODULE_QRC)
    unset(MODULE_BIG_QRC)
    unset(MODULE_QML_IMPORT)
    unset(MODULE_QMLEXT_IMPORT)
    set(MODULE_USE_PCH ON)
    set(MODULE_USE_UNITY ON)
    unset(MODULE_IS_STUB)
    set(MODULE_USE_COVERAGE ON)
endmacro()

macro(declare_thirdparty_module name)
    declare_module(${name})
    set(MODULE_USE_QT OFF)
    set(MODULE_LINK_GLOBAL OFF)
    set(MODULE_USE_PCH OFF)
    set(MODULE_USE_COVERAGE OFF)
endmacro()


macro(add_qml_import_path input_var)
    if (NOT ${${input_var}} STREQUAL "")
        set(QML_IMPORT_PATH "$CACHE{QML_IMPORT_PATH}")
        list(APPEND QML_IMPORT_PATH ${${input_var}})
        list(REMOVE_DUPLICATES QML_IMPORT_PATH)
        set(QML_IMPORT_PATH "${QML_IMPORT_PATH}" CACHE STRING
            "QtCreator extra import paths for QML modules" FORCE)
    endif()
endmacro()

function(target_precompile_headers_clang_ccache target)
    target_precompile_headers(${target} ${ARGN})

    # https://discourse.cmake.org/t/ccache-clang-and-fno-pch-timestamp/7253
    if (CC_IS_CLANG AND CCACHE_PROGRAM)
        target_compile_options(${target} PRIVATE 
            "$<$<COMPILE_LANGUAGE:CXX>:SHELL:-Xclang -fno-pch-timestamp>"
        )
    endif()
endfunction()

macro(setup_module)
    if (MODULE_IS_STUB)
        message(STATUS "Configuring ${MODULE} <${MODULE_ALIAS}> [stub]")
    else()
        message(STATUS "Configuring ${MODULE} <${MODULE_ALIAS}>")
    endif()

    if (MODULE_USE_QT AND QT_SUPPORT)
        # STATIC/SHARED based on BUILD_SHARED_LIBS, which is set in SetupBuildEnvironment.cmake
        qt_add_library(${MODULE} ${MODULE_SRC})
    else()
        # STATIC/SHARED based on BUILD_SHARED_LIBS, which is set in SetupBuildEnvironment.cmake
        add_library(${MODULE} ${MODULE_SRC})
    endif()

    if (MODULE_ALIAS)
        add_library(${MODULE_ALIAS} ALIAS ${MODULE})
    endif()

    if (MODULE_USE_QT AND QT_SUPPORT)
        if (MODULE_QRC)
            qt_add_resources(RCC_SOURCES ${MODULE_QRC})
            target_sources(${MODULE} PRIVATE ${RCC_SOURCES})
        endif()

        if (MODULE_BIG_QRC)
            qt_add_big_resources(RCC_BIG_SOURCES ${MODULE_BIG_QRC})
            target_sources(${MODULE} PRIVATE ${RCC_BIG_SOURCES})
        endif()
    else()
        set_target_properties(${MODULE} PROPERTIES
            AUTOMOC OFF
            AUTOUIC OFF
            AUTORCC OFF
        )
    endif()

    add_qml_import_path(MODULE_QML_IMPORT)
    add_qml_import_path(MODULE_QMLAPI_IMPORT)

    if (BUILD_SHARED_LIBS)
        install(TARGETS ${MODULE} DESTINATION ${SHARED_LIBS_INSTALL_DESTINATION})
    endif()

    if (NOT MUSE_FRAMEWORK_PATH)
        set(MUSE_FRAMEWORK_PATH ${PROJECT_SOURCE_DIR})
    endif()

    if (MUSE_COMPILE_USE_PCH AND MODULE_USE_PCH)
        if (${MODULE} STREQUAL muse_global)
            target_precompile_headers_clang_ccache(${MODULE} PRIVATE ${MUSE_FRAMEWORK_PATH}/buildscripts/pch/pch.h)
        else()
            target_precompile_headers_clang_ccache(${MODULE} REUSE_FROM muse_global)
            target_compile_definitions(${MODULE} PRIVATE muse_global_EXPORTS=1)

            set(MODULE_LINK_GLOBAL ON)
        endif()
    endif()

    if (MUSE_COMPILE_USE_UNITY)
        if (MODULE_USE_UNITY)
            set_target_properties(${MODULE} PROPERTIES UNITY_BUILD ON)
        else()
            set_target_properties(${MODULE} PROPERTIES UNITY_BUILD OFF)
        endif()
    endif()

    target_include_directories(${MODULE} PUBLIC
        ${MODULE_INCLUDE}
    )

    target_include_directories(${MODULE} PRIVATE
        ${PROJECT_BINARY_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}
        ${MODULE_ROOT}

        ${PROJECT_SOURCE_DIR}/src

        ${MUSE_FRAMEWORK_PATH}
        ${MUSE_FRAMEWORK_PATH}/framework
        ${MUSE_FRAMEWORK_PATH}/framework/global
        ${MUSE_FRAMEWORK_PATH}/framework/testing/thirdparty/googletest/googletest/include

        # compat
        ${MUSE_FRAMEWORK_PATH}/src
        ${MUSE_FRAMEWORK_PATH}/src/framework
        ${MUSE_FRAMEWORK_PATH}/src/framework/global
        ${MUSE_FRAMEWORK_PATH}/src/framework/testing/thirdparty/googletest/googletest/include
        # end compat

        ${MODULE_INCLUDE_PRIVATE}
    )

    target_compile_definitions(${MODULE} PUBLIC
        ${MODULE_DEF}
        ${MODULE}_QML_IMPORT="${MODULE_QML_IMPORT}"
    )

    if (MUSE_ENABLE_UNIT_TESTS_CODE_COVERAGE AND MODULE_USE_COVERAGE)
        set(COVERAGE_FLAGS -fprofile-arcs -ftest-coverage --coverage)
        target_compile_options(${MODULE} PRIVATE ${COVERAGE_FLAGS})
        target_link_options(${MODULE} PRIVATE -lgcov --coverage)
    endif()

    if (NOT ${MODULE} MATCHES muse_global AND MODULE_LINK_GLOBAL)
        target_link_libraries(${MODULE} PRIVATE muse_global)
    endif()

    target_link_libraries(${MODULE}
        PRIVATE ${MODULE_LINK} ${COVERAGE_FLAGS}
        PUBLIC ${MODULE_LINK_PUBLIC}
    )
endmacro()
