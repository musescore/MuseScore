#=============================================================================
#  BWW to MusicXML converter
#  Part of MusE Score
#  Linux Music Score Editor
#
#  Copyright (C) 2002-2010 by Werner Schweer and others
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

if (APPLE)
        file(GLOB_RECURSE INCS "*.h")
else (APPLE)
        set(INCS "")
endif (APPLE)

set (BWW_DIR ${PROJECT_SOURCE_DIR}/importexport/bww)

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

   target_include_directories(bww2mxml
      PRIVATE
         ${BWW_DIR}
      )

   target_link_libraries(bww2mxml
      ${QT_LIBRARIES}
      )

   if (BUILD_PCH)
         target_use_pch(bww2mxml)
   endif (BUILD_PCH)
endif (NOT MINGW AND NOT APPLE AND NOT MSVC)
