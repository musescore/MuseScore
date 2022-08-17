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

if (NOT BUILD_SHORTCUTS_MODULE)
    add_subdirectory(shortcuts)
endif (NOT BUILD_SHORTCUTS_MODULE)

if (NOT BUILD_NETWORK_MODULE)
add_subdirectory(network)
endif (NOT BUILD_NETWORK_MODULE)

if (NOT BUILD_AUDIO_MODULE)
    add_subdirectory(audio)
endif (NOT BUILD_AUDIO_MODULE)

if (NOT BUILD_VST)
    add_subdirectory(vst)
endif (NOT BUILD_VST)
