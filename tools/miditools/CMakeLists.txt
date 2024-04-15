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

add_executable (smf2xml smf2xml.cpp xmlwriter.cpp midifile.cpp)

if (NOT MSVC)
   set_target_properties(smf2xml
      PROPERTIES COMPILE_FLAGS "-g -Wall -Wextra"
      )
else (NOT MSVC)
   set_target_properties (smf2xml
      PROPERTIES COMPILE_FLAGS ""
      )
endif (NOT MSVC)   

target_link_libraries(smf2xml
      ${QT_LIBRARIES}
      )

add_executable (xml2smf xml2smf.cpp xmlreader.cpp midifile.cpp)

if (NOT MSVC)
   set_target_properties (
      xml2smf
      PROPERTIES COMPILE_FLAGS "-g -Wall -Wextra"
      )
else (NOT MSVC)
   set_target_properties (
      xml2smf
      PROPERTIES COMPILE_FLAGS ""
      )
endif (NOT MSVC)   

target_link_libraries(xml2smf
      ${QT_LIBRARIES}
      )

