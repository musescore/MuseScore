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

#add_subdirectory(global)
add_subdirectory(ui)
add_subdirectory(uicomponents)
add_subdirectory(fonts)
add_subdirectory(actions)
add_subdirectory(accessibility)

if (BUILD_SHORTCUTS_MODULE)
    add_subdirectory(shortcuts)
endif (BUILD_SHORTCUTS_MODULE)

if (BUILD_NETWORK_MODULE)
    add_subdirectory(network)
endif (BUILD_NETWORK_MODULE)

if (BUILD_SYSTEM_MODULE)
    add_subdirectory(system)
endif (BUILD_SYSTEM_MODULE)

if (BUILD_AUDIO_MODULE)
    add_subdirectory(audio)
endif (BUILD_AUDIO_MODULE)

add_subdirectory(midi)

if (BUILD_TELEMETRY_MODULE)
      add_subdirectory(telemetry)
endif (BUILD_TELEMETRY_MODULE)

if (BUILD_UNIT_TESTS)
    add_subdirectory(global/tests)
    add_subdirectory(system/tests)
    add_subdirectory(ui/tests)
endif(BUILD_UNIT_TESTS)

if (BUILD_VST)
    add_subdirectory(vst)
endif(BUILD_VST)
