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

if (NOT MUE_BUILD_BRAILLE_MODULE)
    add_subdirectory(braille)
endif()

if (NOT MUE_BUILD_INSPECTOR_MODULE)
    add_subdirectory(inspector)
endif()

if (NOT MUE_BUILD_INSTRUMENTSSCENE_MODULE)
    add_subdirectory(instrumentsscene)
endif()

if (NOT MUE_BUILD_NOTATION_MODULE)
    add_subdirectory(notation)
endif()

if (NOT MUE_BUILD_PALETTE_MODULE)
    add_subdirectory(palette)
endif()

if (NOT MUE_BUILD_PLAYBACK_MODULE)
    add_subdirectory(playback)
endif()

if (NOT MUE_BUILD_PROJECT_MODULE)
    add_subdirectory(project)
endif()

if (NOT MUSE_MODULE_WORKSPACE)
    add_subdirectory(workspacescene)
endif()

if (NOT MUE_BUILD_MUSESOUNDS_MODULE)
    add_subdirectory(musesounds)
endif()
