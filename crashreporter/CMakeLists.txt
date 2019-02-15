#=============================================================================
#  MuseScore
#  Music Composition & Notation
#
#  Copyright (C) 2019 Werner Schweer and others
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#=============================================================================

QT5_WRAP_UI(ui_headers crashreporter.ui)

add_executable(mscore-crash-reporter
      ${ui_headers}
      ${_all_h_file}
      crashreporter.h
      crashreporter.cpp
      )

set_target_properties(
      mscore-crash-reporter
      PROPERTIES
      OUTPUT_NAME ${CRASHREPORTER_EXECUTABLE_NAME}
      )

target_include_directories(mscore-crash-reporter PRIVATE
      ${PROJECT_BINARY_DIR} # config.h
      )

target_link_libraries(mscore-crash-reporter ${QT_LIBRARIES})

install(TARGETS mscore-crash-reporter RUNTIME DESTINATION bin)
