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

add_subdirectory(bb)
add_subdirectory(braille)
add_subdirectory(bww)
add_subdirectory(capella)
add_subdirectory(midi)
add_subdirectory(musedata)
add_subdirectory(musicxml)
add_subdirectory(ove)
add_subdirectory(audioexport)
add_subdirectory(imagesexport)

# We have two implementations for importing `gp` files
# Old and new
# The new one is more structured, imports more correctly, more amenable to improvements

add_subdirectory(guitarpro)
#add_subdirectory(guitarpro_old)
