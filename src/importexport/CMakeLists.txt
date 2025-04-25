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

if(MUE_BUILD_IMPORTEXPORT_MODULE)
    add_subdirectory(bb)

    add_subdirectory(bww)
    add_subdirectory(capella)
    add_subdirectory(midi)
    add_subdirectory(musedata)
    add_subdirectory(musicxml)
    add_subdirectory(ove)
    add_subdirectory(audioexport)
    add_subdirectory(imagesexport)
    add_subdirectory(guitarpro)
    add_subdirectory(mei)
    add_subdirectory(lyrics)

    if(MUE_BUILD_VIDEOEXPORT_MODULE)
        add_subdirectory(videoexport)
    endif()
else()
    if(MUE_BUILD_IMAGESEXPORT_MODULE)
        add_subdirectory(imagesexport)
    endif()
endif()
