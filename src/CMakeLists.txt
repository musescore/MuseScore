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

# Modules must be listed in dependency order
if (MUE_BUILD_BRAILLE_MODULE)
    add_subdirectory(braille)
else()
    add_subdirectory(stubs/braille)
endif()

add_subdirectory(context)

if (MUE_BUILD_CONVERTER_MODULE)
    add_subdirectory(converter)
endif()

add_subdirectory(engraving)

add_subdirectory(importexport)

if (MUE_BUILD_INSPECTOR_MODULE)
    add_subdirectory(inspector)
else()
    add_subdirectory(stubs/inspector)
endif()

if (MUE_BUILD_INSTRUMENTSSCENE_MODULE)
    add_subdirectory(instrumentsscene)
else()
    add_subdirectory(stubs/instrumentsscene)
endif()

if (MUE_BUILD_MUSESOUNDS_MODULE)
    add_subdirectory(musesounds)
else()
    add_subdirectory(stubs/musesounds)
endif()

if (MUE_BUILD_NOTATION_MODULE)
    add_subdirectory(notation)
else()
    add_subdirectory(stubs/notation)
endif()

if (MUE_BUILD_NOTATIONSCENE_MODULE)
    add_subdirectory(notationscene)
else()
    add_subdirectory(stubs/notationscene)
endif()

if (MUE_BUILD_PALETTE_MODULE)
    add_subdirectory(palette)
else()
    add_subdirectory(stubs/palette)
endif()

if (MUE_BUILD_PLAYBACK_MODULE)
    add_subdirectory(playback)
else()
    add_subdirectory(stubs/playback)
endif()

if (MUE_BUILD_PREFERENCES_MODULE)
    add_subdirectory(preferences)
endif()

if (MUE_BUILD_PRINT_MODULE)
    add_subdirectory(print)
endif()

if (MUE_BUILD_PROJECT_MODULE)
    add_subdirectory(project)
endif()

if (MUE_BUILD_APPSHELL_MODULE)
    if (MUE_CONFIGURATION_IS_APPWEB)
        add_subdirectory(web/appshell)
    else()
        add_subdirectory(appshell)
    endif()
endif()

# App (main)
add_subdirectory(app)
if (MUE_CONFIGURATION_IS_APPWEB)
    add_subdirectory(web/appjs)
endif()

if (MUE_BUILD_MACOS_INTEGRATION AND OS_IS_MAC)
    add_subdirectory(macos_integration)
endif()
