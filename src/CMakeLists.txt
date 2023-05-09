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

# App (main)
add_subdirectory(app)

# Framework
add_subdirectory(framework)

# Modules
if (MUE_BUILD_APPSHELL_MODULE)
    add_subdirectory(appshell)
endif()

if (MUE_BUILD_AUTOBOT_MODULE)
    add_subdirectory(autobot)
endif()

if (MUE_BUILD_BRAILLE_MODULE)
    add_subdirectory(braille)
endif()

if (MUE_BUILD_CLOUD_MODULE)
    add_subdirectory(cloud)
endif()

add_subdirectory(commonscene)
add_subdirectory(context)

if (MUE_BUILD_CONVERTER_MODULE)
    add_subdirectory(converter)
endif()

if (MUE_BUILD_DIAGNOSTICS_MODULE)
    add_subdirectory(diagnostics)
endif()

add_subdirectory(engraving)

add_subdirectory(importexport)

if (MUE_BUILD_INSPECTOR_MODULE)
    add_subdirectory(inspector)
endif()

if (MUE_BUILD_INSTRUMENTSSCENE_MODULE)
    add_subdirectory(instrumentsscene)
endif()

if (MUE_BUILD_LANGUAGES_MODULE)
    add_subdirectory(languages)
endif()

if (MUE_BUILD_LEARN_MODULE)
    add_subdirectory(learn)
endif()

if (MUE_BUILD_MULTIINSTANCES_MODULE)
    add_subdirectory(multiinstances)
endif()

if (MUE_BUILD_NOTATION_MODULE)
    add_subdirectory(notation)
endif()

if (MUE_BUILD_PALETTE_MODULE)
    add_subdirectory(palette)
endif()

if (MUE_BUILD_PIANOROLL_MODULE)
    add_subdirectory(pianoroll)
endif()

if (MUE_BUILD_PLAYBACK_MODULE)
    add_subdirectory(playback)
endif()

if (MUE_BUILD_PLUGINS_MODULE)
    add_subdirectory(plugins)
endif()

add_subdirectory(print)

if (MUE_BUILD_PROJECT_MODULE)
    add_subdirectory(project)
endif()

if (MUE_BUILD_UPDATE_MODULE)
    add_subdirectory(update)
endif()

if (MUE_BUILD_WORKSPACE_MODULE)
    add_subdirectory(workspace)
endif()

# Stubs
add_subdirectory(stubs)

if (OS_IS_WASM)
    add_subdirectory(wasmtest)
endif()
