# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2024 MuseScore BVBA and others
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
# set(MODULE_DEF ...)                         - set definitions
# set(MODULE_SRC ...)                         - set sources and headers files
# set(MODULE_LINK ...)                        - set libraries for link
# set(MODULE_NOT_LINK_GLOBAL ON)              - set for not link global lib
# set(MODULE_QRC somename.qrc)                - set resource (qrc) file
# set(MODULE_BIG_QRC somename.qrc)            - set big resource (qrc) file
# set(MODULE_UI ...)                          - set ui headers
# set(MODULE_QML_IMPORT ...)                  - set Qml import for QtCreator (so that there is code highlighting, jump, etc.)
# set(MODULE_QMLEXT_IMPORT ...)               - set Qml extensions import for QtCreator (so that there is code highlighting, jump, etc.)
# set(MODULE_USE_PCH_DISABLED ON)                 - set for disable PCH for module
# set(MODULE_USE_UNITY_NONE ON)               - set for disable UNITY BUILD for module
# set(MODULE_OVERRIDDEN_PCH ...)              - set additional precompiled headers required for module
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
    unset(MODULE_NOT_LINK_GLOBAL)
    unset(MODULE_QRC)
    unset(MODULE_BIG_QRC)
    unset(MODULE_UI)
    unset(MODULE_QML_IMPORT)
    unset(MODULE_QMLEXT_IMPORT)
    unset(MODULE_USE_PCH_DISABLED)
    unset(MODULE_USE_UNITY_NONE)
    unset(MODULE_OVERRIDDEN_PCH)
    unset(MODULE_IS_STUB)
endmacro()


macro(setup_module)

    if (MODULE_IS_STUB)
        message(STATUS "Configuring ${MODULE} <${MODULE_ALIAS}> [stub]")
    else()
        message(STATUS "Configuring ${MODULE} <${MODULE_ALIAS}>")
    endif()

    if (NOT MUSE_FRAMEWORK_PATH)
        set(MUSE_FRAMEWORK_PATH ${PROJECT_SOURCE_DIR})
    endif()

    if (MODULE_QRC AND NOT NO_QT_SUPPORT)
        if (MUE_COMPILE_QT5_COMPAT)
            qt5_add_resources(RCC_SOURCES ${MODULE_QRC})
        else()
            qt_add_resources(RCC_SOURCES ${MODULE_QRC})
        endif()
    endif()

    if (MODULE_BIG_QRC AND NOT NO_QT_SUPPORT)
        if (MUE_COMPILE_QT5_COMPAT)
            qt5_add_big_resources(RCC_BIG_SOURCES ${MODULE_BIG_QRC})
        else()
            qt_add_big_resources(RCC_BIG_SOURCES ${MODULE_BIG_QRC})
        endif()
    endif()

    if (MODULE_UI)
        if (MUE_COMPILE_QT5_COMPAT)
            find_package(Qt5Widgets)
            QT5_WRAP_UI(ui_headers ${MODULE_UI} )
        else()
            find_package(Qt6Widgets)
            QT6_WRAP_UI(ui_headers ${MODULE_UI} )
        endif()
    endif()

    if (NOT ${MODULE_QML_IMPORT} STREQUAL "")
        set(QML_IMPORT_PATH "${QML_IMPORT_PATH};${MODULE_QML_IMPORT}" CACHE STRING "QtCreator extra import paths for QML modules" FORCE)
    endif()

    if (NOT ${MODULE_QMLAPI_IMPORT} STREQUAL "")
        set(QML_IMPORT_PATH "${QML_IMPORT_PATH};${MODULE_QMLAPI_IMPORT}" CACHE STRING "QtCreator extra import paths for QML modules" FORCE)
    endif()

    if (CC_IS_EMSCRIPTEN)
        add_library(${MODULE} OBJECT)
    else()
        add_library(${MODULE}) # STATIC/SHARED set global in the SetupBuildEnvironment.cmake
    endif()

    if (MODULE_ALIAS)
        add_library(${MODULE_ALIAS} ALIAS ${MODULE})
    endif()

    if (BUILD_SHARED_LIBS)
        install(TARGETS ${MODULE} DESTINATION ${SHARED_LIBS_INSTALL_DESTINATION})

        if (NOT MSVC)
            set_target_properties(${MODULE} PROPERTIES COMPILE_FLAGS "-fPIC")
        endif (NOT MSVC)
    endif()

    if (MUSE_COMPILE_USE_PCH)
        if (MODULE_USE_PCH_DISABLED)
            # disabled pch for current module
        else()
            if (NOT ${MODULE} MATCHES muse_global)
                if (NOT DEFINED MODULE_OVERRIDDEN_PCH)
                    target_precompile_headers(${MODULE} REUSE_FROM muse_global)
                    target_compile_definitions(${MODULE} PRIVATE muse_global_EXPORTS=1)
                else()
                    target_precompile_headers(${MODULE} PRIVATE ${MODULE_OVERRIDDEN_PCH})
                endif()
                if (MODULE_NOT_LINK_GLOBAL)
                    set(MODULE_NOT_LINK_GLOBAL OFF)
                endif()
            else()
                target_precompile_headers(${MODULE} PRIVATE ${MUSE_FRAMEWORK_PATH}/build/pch/pch.h)
            endif()
        endif()
    endif() # MUSE_COMPILE_USE_PCH

    if (MUE_COMPILE_USE_UNITY)
        if (MODULE_USE_UNITY_NONE)
            # disabled unity build for current module
            set_target_properties(${MODULE} PROPERTIES UNITY_BUILD OFF)
        else()
            set_target_properties(${MODULE} PROPERTIES UNITY_BUILD ON)
        endif()
    endif(MUE_COMPILE_USE_UNITY)

    target_sources(${MODULE} PRIVATE
        ${ui_headers}
        ${RCC_SOURCES}
        ${RCC_BIG_SOURCES}
        ${MODULE_SRC}
        )

    target_include_directories(${MODULE} PUBLIC
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

        ${MODULE_INCLUDE}
    )

    target_compile_definitions(${MODULE} PUBLIC
        ${MODULE_DEF}
        ${MODULE}_QML_IMPORT="${MODULE_QML_IMPORT}"
    )

    if (NOT ${MODULE} MATCHES muse_global)
        if (NOT MODULE_NOT_LINK_GLOBAL)
            set(MODULE_LINK muse_global ${MODULE_LINK})
        endif()
    endif()

    set(MODULE_LINK ${CMAKE_DL_LIBS} ${QT_LIBRARIES} ${MODULE_LINK})

    target_link_libraries(${MODULE} PRIVATE ${MODULE_LINK} )

endmacro()
