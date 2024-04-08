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

if (NOT MUSE_MODULE_ACCESSIBILITY)
    add_subdirectory(accessibility)
endif()

if (NOT MUSE_MODULE_AUDIO)
    add_subdirectory(audio)
endif()

if (NOT MUSE_MODULE_CLOUD)
    add_subdirectory(cloud)
endif()

if (NOT MUSE_MODULE_LANGUAGES)
    add_subdirectory(languages)
endif()

if (NOT MUSE_MODULE_LEARN)
    add_subdirectory(learn)
endif()

if (NOT MUSE_MODULE_MIDI)
    add_subdirectory(midi)
endif()

if (NOT MUSE_MODULE_MPE)
    add_subdirectory(mpe)
endif()

if (NOT MUSE_MODULE_MULTIINSTANCES)
    add_subdirectory(multiinstances)
endif()

if (NOT MUSE_MODULE_NETWORK)
    add_subdirectory(network)
endif()

if (NOT MUSE_MODULE_SHORTCUTS)
    add_subdirectory(shortcuts)
endif()

if (NOT MUSE_MODULE_UPDATE)
    add_subdirectory(update)
endif()

if (NOT MUSE_MODULE_VST)
    add_subdirectory(vst)
endif()

if (NOT MUSE_MODULE_WORKSPACE)
    add_subdirectory(workspace)
endif()
