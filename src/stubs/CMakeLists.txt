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

add_subdirectory(framework)

if (NOT MUE_BUILD_CLOUD_MODULE)
    add_subdirectory(cloud)
endif()

if (NOT MUE_BUILD_INSPECTOR_MODULE)
    add_subdirectory(inspector)
endif()

if (NOT MUE_BUILD_INSTRUMENTSSCENE_MODULE)
    add_subdirectory(instrumentsscene)
endif()

if (NOT MUE_BUILD_LANGUAGES_MODULE)
    add_subdirectory(languages)
endif()

if (NOT MUE_BUILD_LEARN_MODULE)
    add_subdirectory(learn)
endif()

if (NOT MUE_BUILD_MULTIINSTANCES_MODULE)
    add_subdirectory(multiinstances)
endif()

if (NOT MUE_BUILD_NOTATION_MODULE)
    add_subdirectory(notation)
endif()

if (NOT MUE_BUILD_PALETTE_MODULE)
    add_subdirectory(palette)
endif()

if (NOT MUE_BUILD_PIANOROLL_MODULE)
    add_subdirectory(pianoroll)
endif()

if (NOT MUE_BUILD_PLAYBACK_MODULE)
    add_subdirectory(playback)
endif()

if (NOT MUE_BUILD_PIANOROLL_MODULE)
    add_subdirectory(pianoroll)
endif()

if (NOT MUE_BUILD_PLUGINS_MODULE)
    add_subdirectory(plugins)
endif()

if (NOT MUE_BUILD_PROJECT_MODULE)
    add_subdirectory(project)
endif()

if (NOT MUE_BUILD_UPDATE_MODULE)
    add_subdirectory(update)
endif()

if (NOT MUE_BUILD_WORKSPACE_MODULE)
    add_subdirectory(workspace)
endif()
