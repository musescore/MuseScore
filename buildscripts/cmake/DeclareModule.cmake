# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2026 MuseScore Limited
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
# along with this program. If not, see <https://www.gnu.org/licenses/>.

### LEGACY MACROS
# TODO: remove this file after migrating all modules to muse_create_module

## Declare
# declare_module(somename) - set module (target) name

## Setup
# set(MODULE somename)                        - set module (target) name
# set(MODULE_ALIAS somename)                  - set module (target) alias name
# set(MODULE_INCLUDE ...)                     - set include (by default see below include_directories)
# set(MODULE_INCLUDE_PRIVATE ...)             - set private include
# set(MODULE_DEF ...)                         - set definitions
# set(MODULE_DEF_PRIVATE ...)                 - set private definitions
# set(MODULE_SRC ...)                         - set sources and headers files
# set(MODULE_LINK ...)                        - set libraries for link
# set(MODULE_QRC somename.qrc)                - set resource (qrc) file
# set(MODULE_BIG_QRC somename.qrc)            - set big resource (qrc) file
# set(MODULE_QML_IMPORT ...)                  - set Qml import for QtCreator (so that there is code highlighting, jump, etc.)
# set(MODULE_QMLAPI_IMPORT ...)               - set Qml api import for QtCreator (so that there is code highlighting, jump, etc.)
# set(MODULE_QMLEXT_IMPORT ...)               - set Qml extensions import for QtCreator (so that there is code highlighting, jump, etc.)
# set(MODULE_IS_STUB ON)                      - set a mark that the module is stub

# After all the settings you need to do:
# setup_module()

macro(declare_module name)
    set(MODULE ${name})
    # just reset all settings
    unset(MODULE_ALIAS)
    unset(MODULE_INCLUDE)
    unset(MODULE_DEF)
    unset(MODULE_SRC)
    unset(MODULE_LINK)
    set(MODULE_USE_QT ON)
    unset(MODULE_QRC)
    unset(MODULE_BIG_QRC)
    unset(MODULE_QML_IMPORT)
    unset(MODULE_QMLAPI_IMPORT)
    unset(MODULE_QMLEXT_IMPORT)
    unset(MODULE_IS_STUB)
endmacro()

macro(add_qml_import_path_if_not_empty input_var)
    if (${input_var})
        add_qml_import_path(${${input_var}})
    endif()
endmacro()

macro(setup_module)
    set(ARGS)

    if (NOT MODULE_USE_QT)
        list(APPEND ARGS NO_QT)
    endif()

    if (MODULE_IS_STUB)
        list(APPEND ARGS STUB)
    endif()

    if (MODULE_ALIAS)
        list(APPEND ARGS ALIAS ${MODULE_ALIAS})
    endif()

    muse_create_module(${MODULE} ${ARGS})

    target_sources(${MODULE} PRIVATE ${MODULE_SRC})

    if (MODULE_USE_QT AND MUSE_QT_SUPPORT)
        if (MODULE_QRC)
            qt_add_resources(RCC_SOURCES ${MODULE_QRC})
            target_sources(${MODULE} PRIVATE ${RCC_SOURCES})
        endif()

        if (MODULE_BIG_QRC)
            qt_add_big_resources(RCC_BIG_SOURCES ${MODULE_BIG_QRC})
            target_sources(${MODULE} PRIVATE ${RCC_BIG_SOURCES})
        endif()
    endif()

    add_qml_import_path_if_not_empty(MODULE_QML_IMPORT)
    add_qml_import_path_if_not_empty(MODULE_QMLAPI_IMPORT)
    add_qml_import_path_if_not_empty(MODULE_QMLEXT_IMPORT)

    if (BUILD_SHARED_LIBS)
        install(TARGETS ${MODULE} DESTINATION ${SHARED_LIBS_INSTALL_DESTINATION})
    endif()

    if (NOT MUSE_FRAMEWORK_PATH)
        set(MUSE_FRAMEWORK_PATH ${PROJECT_SOURCE_DIR})
    endif()

    target_include_directories(${MODULE}
        PRIVATE ${MODULE_INCLUDE_PRIVATE}
        PUBLIC ${MODULE_INCLUDE}
    )

    target_compile_definitions(${MODULE} PUBLIC
        ${MODULE_DEF}
        ${MODULE}_QML_IMPORT="${MODULE_QML_IMPORT}"
    )

    target_compile_definitions(${MODULE} PRIVATE
        ${MODULE_DEF_PRIVATE}
    )

    target_link_libraries(${MODULE} PRIVATE ${MODULE_LINK})
endmacro()
