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

if (APPLE)
        file(GLOB_RECURSE INCS "*.h")
else (APPLE)
        set(INCS "")
endif (APPLE)

set (BWW_DIR ${PROJECT_SOURCE_DIR}/mu4/importexport/internal/bww)

include_directories(
      ${BWW_DIR}
      )

if (NOT MINGW AND NOT APPLE AND NOT MSVC)
   add_executable(bww2mxml
      ${INCS}
      ${BWW_SRC}
      main.cpp
      mxmlwriter.cpp
      ${BWW_DIR}/writer.cpp
      ${BWW_DIR}/lexer.cpp
      ${BWW_DIR}/parser.cpp
      ${BWW_DIR}/symbols.cpp
      )

   set_target_properties (
         bww2mxml
         PROPERTIES
            COMPILE_FLAGS "-x c++ -g -Wall -Wextra"
         )

   target_link_libraries(bww2mxml
      ${QT_LIBRARIES}
      )

endif (NOT MINGW AND NOT APPLE AND NOT MSVC)

