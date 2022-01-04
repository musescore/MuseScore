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

if (NOT BUILD_LEARN_MODULE)
    add_subdirectory(learn)
endif (NOT BUILD_LEARN_MODULE)

if (NOT BUILD_WORKSPACE_MODULE)
    add_subdirectory(workspace)
endif (NOT BUILD_WORKSPACE_MODULE)

if (NOT BUILD_CLOUD_MODULE)
    add_subdirectory(cloud)
endif (NOT BUILD_CLOUD_MODULE)

if (NOT BUILD_EXTENSIONS_MODULE)
    add_subdirectory(extensions)
endif (NOT BUILD_EXTENSIONS_MODULE)

if (NOT BUILD_LANGUAGES_MODULE)
    add_subdirectory(languages)
endif (NOT BUILD_LANGUAGES_MODULE)

if (NOT BUILD_PLUGINS_MODULE)
    add_subdirectory(plugins)
endif (NOT BUILD_PLUGINS_MODULE)

if (NOT BUILD_PLAYBACK_MODULE)
    add_subdirectory(playback)
endif (NOT BUILD_PLAYBACK_MODULE)

if (NOT BUILD_PALETTE_MODULE)
    add_subdirectory(palette)
endif (NOT BUILD_PALETTE_MODULE)

if (NOT BUILD_INSTRUMENTSSCENE_MODULE)
    add_subdirectory(instrumentsscene)
endif (NOT BUILD_INSTRUMENTSSCENE_MODULE)

if (NOT BUILD_INSPECTOR_MODULE)
    add_subdirectory(inspector)
endif (NOT BUILD_INSPECTOR_MODULE)

if (NOT BUILD_MULTIINSTANCES_MODULE)
    add_subdirectory(multiinstances)
endif()
