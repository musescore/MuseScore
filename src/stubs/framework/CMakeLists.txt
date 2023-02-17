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

if (NOT MUE_BUILD_ACCESSIBILITY_MODULE)
    add_subdirectory(accessibility)
endif()

if (NOT MUE_BUILD_AUDIO_MODULE)
    add_subdirectory(audio)
endif()

if (NOT MUE_BUILD_MIDI_MODULE)
    add_subdirectory(midi)
endif()

if (NOT MUE_BUILD_MPE_MODULE)
    add_subdirectory(mpe)
endif()

if (NOT MUE_BUILD_NETWORK_MODULE)
    add_subdirectory(network)
endif()

if (NOT MUE_BUILD_SHORTCUTS_MODULE)
    add_subdirectory(shortcuts)
endif()

if (NOT MUE_BUILD_VST_MODULE)
    add_subdirectory(vst)
endif()
