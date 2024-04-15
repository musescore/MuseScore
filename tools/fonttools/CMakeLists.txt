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

set(CMAKE_VERBOSE_MAKEFILE OFF)

add_executable(
      genft
      genft.cpp
      )

target_link_libraries(genft ${QT_LIBRARIES} -lfreetype)

if (NOT MSVC)
   set_target_properties(genft PROPERTIES COMPILE_FLAGS "-I/usr/include/freetype2 -g -Wall -Wextra -Winvalid-pch")
else (NOT MSVC)
   set_target_properties (
      genft
      PROPERTIES
         COMPILE_FLAGS ""     # Might be enough, have to check if a forced directory or forced include is needed.
      )
endif (NOT MSVC)   

