# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore BVBA and others
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


# Main
add_subdirectory(main)

# Framework
add_subdirectory(framework)

# Domain model
add_subdirectory(engraving)

# AppShell
add_subdirectory(appshell)

# Modules
add_subdirectory(context)
add_subdirectory(notation)
add_subdirectory(project)

if (BUILD_LEARN_MODULE)
    add_subdirectory(learn)
endif (BUILD_LEARN_MODULE)

add_subdirectory(converter)

if (BUILD_WORKSPACE_MODULE)
    add_subdirectory(workspace)
endif (BUILD_WORKSPACE_MODULE)

if (NOT OS_IS_WASM)
    add_subdirectory(importexport)

    add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/beatroot beatroot) # for midi import
    add_subdirectory(${PROJECT_SOURCE_DIR}/thirdparty/rtf2html rtf2html) # for capella

    if (BUILD_CLOUD_MODULE)
        add_subdirectory(cloud)
    endif (BUILD_CLOUD_MODULE)

    if (BUILD_EXTENSIONS_MODULE)
        add_subdirectory(extensions)
    endif (BUILD_EXTENSIONS_MODULE)

    if (BUILD_LANGUAGES_MODULE)
        add_subdirectory(languages)
    endif (BUILD_LANGUAGES_MODULE)

    if (BUILD_PLUGINS_MODULE)
        add_subdirectory(plugins)
    endif (BUILD_PLUGINS_MODULE)
endif()

# Scenes common
add_subdirectory(commonscene)
if (BUILD_PLAYBACK_MODULE)
    add_subdirectory(playback)
endif (BUILD_PLAYBACK_MODULE)

# Notation
if (BUILD_PALETTE_MODULE)
    add_subdirectory(palette)
endif (BUILD_PALETTE_MODULE)
if (BUILD_INSTRUMENTSSCENE_MODULE)
    add_subdirectory(instrumentsscene)
endif (BUILD_INSTRUMENTSSCENE_MODULE)
if (BUILD_INSPECTOR_MODULE)
    add_subdirectory(inspector)
endif (BUILD_INSPECTOR_MODULE)

# Other
if (BUILD_MULTIINSTANCES_MODULE)
    add_subdirectory(multiinstances)
endif()

add_subdirectory(diagnostics)

if (BUILD_AUTOBOT_MODULE)
    add_subdirectory(autobot)
endif()

# Stubs
add_subdirectory(stubs)

if (BUILD_UNIT_TESTS)
#    add_subdirectory(notation/tests) no tests at moment
    add_subdirectory(project/tests)

    add_subdirectory(engraving/tests)
    add_subdirectory(engraving/utests)
    add_subdirectory(importexport/bb/tests)
    add_subdirectory(importexport/braille/tests)
    add_subdirectory(importexport/bww/tests)
    add_subdirectory(importexport/capella/tests)
#    add_subdirectory(importexport/guitarpro/tests)
    add_subdirectory(importexport/midi/tests)
    add_subdirectory(importexport/musicxml/tests)
endif(BUILD_UNIT_TESTS)

if (OS_IS_WASM)
    add_subdirectory(wasmtest)
endif()
