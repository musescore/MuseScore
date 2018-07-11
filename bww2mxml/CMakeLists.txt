#=============================================================================
#  BWW to MusicXML converter
#  Part of MusE Score
#  Linux Music Score Editor
#  $Id: CMakeLists.txt 5560 2012-04-19 11:39:34Z lasconic $
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

include (${PROJECT_SOURCE_DIR}/build/gch.cmake)

if (APPLE)
        file(GLOB_RECURSE INCS "*.h")
else (APPLE)
        set(INCS "")
endif (APPLE)

add_library(bww STATIC
   ${INCS}
   lexer.cpp
   parser.cpp
   symbols.cpp
   writer.cpp
   )

if (MSVC)
   set_target_properties (
      bww
      PROPERTIES
         COMPILE_FLAGS "${PCH_INCLUDE}"
      )
endif (MSVC)

if (NOT MINGW AND NOT APPLE AND NOT MSVC)
   add_executable(bww2mxml
      ${INCS}
      lexer.cpp
      main.cpp
      mxmlwriter.cpp
      parser.cpp
      symbols.cpp
      writer.cpp
      )

   set_target_properties (
         bww2mxml
         PROPERTIES
            COMPILE_FLAGS "-x c++ -include ${PROJECT_BINARY_DIR}/all.h -g -Wall -Wextra -Winvalid-pch"
         )

   target_link_libraries(bww2mxml
      ${QT_LIBRARIES}
      )
   ADD_DEPENDENCIES(bww2mxml mops1)
   ADD_DEPENDENCIES(bww2mxml mops2)
endif (NOT MINGW AND NOT APPLE AND NOT MSVC)

xcode_pch(bww all)

# Use MSVC pre-compiled headers
vstudio_pch( bww )

# MSVC does not depend on mops1 & mops2 for PCH
if (NOT MSVC)
   ADD_DEPENDENCIES(bww mops1)
   ADD_DEPENDENCIES(bww mops2)
endif (NOT MSVC)
