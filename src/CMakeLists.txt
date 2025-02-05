# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited
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

# Framework, should be first to work pch
add_subdirectory(framework)

# App (main)
add_subdirectory(app)

# Modules
if (MUE_BUILD_APPSHELL_MODULE)
    add_subdirectory(appshell)
endif()

if (MUE_BUILD_BRAILLE_MODULE)
    add_subdirectory(braille)
endif()

add_subdirectory(commonscene)
add_subdirectory(context)

if (MUE_BUILD_CONVERTER_MODULE)
    add_subdirectory(converter)
endif()

add_subdirectory(engraving)

add_subdirectory(importexport)

if (MUE_BUILD_INSPECTOR_MODULE)
    add_subdirectory(inspector)
endif()

if (MUE_BUILD_INSTRUMENTSSCENE_MODULE)
    add_subdirectory(instrumentsscene)
endif()

if (MUE_BUILD_MUSESOUNDS_MODULE)
    add_subdirectory(musesounds)
endif()

if (MUE_BUILD_NOTATION_MODULE)
    add_subdirectory(notation)
endif()

if (MUE_BUILD_PALETTE_MODULE)
    add_subdirectory(palette)
endif()

if (MUE_BUILD_PLAYBACK_MODULE)
    add_subdirectory(playback)
endif()

add_subdirectory(print)

if (MUE_BUILD_PROJECT_MODULE)
    add_subdirectory(project)
endif()

# Stubs
add_subdirectory(stubs)

if (OS_IS_WASM)
    add_subdirectory(wasmtest)
endif()
