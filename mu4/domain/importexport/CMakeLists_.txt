#=============================================================================
#  MuseScore
#  Music Composition & Notation
#
#  Copyright (C) 2020 MuseScore BVBA and others
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

set(MODULE importexport)

include (${PROJECT_SOURCE_DIR}/build/gch.cmake)

include_directories(
      ${PROJECT_SOURCE_DIR}
      ${CMAKE_CURRENT_BINARY_DIR}
      )

if (NOT MSVC)
   set(_all_h_file "${PROJECT_BINARY_DIR}/all.h")
else (NOT MSVC)
   set(_all_h_file "${PROJECT_SOURCE_DIR}/all.h")
endif (NOT MSVC)

include(${CMAKE_CURRENT_LIST_DIR}/musicxml/musicxml.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/midiimport/midiimport.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/ove/ove.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/guitarpro/guitarpro.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/musedata/musedata.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/bb/bb.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/capella/capella.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/bww/bww.cmake)

add_library (
      ${MODULE} STATIC
      ${_all_h_file}
      ${PCH}
      ${MUSICXML_SRC}
      ${MIDIIMPORT_SRC}
      ${OVE_SRC}
      ${GUITARPRO_SRC}
      ${MUSEDATA_SRC}
      ${BB_SRC}
      ${CAPELLA_SRC}
      ${BWW_SRC}
      )

add_subdirectory(../thirdparty/beatroot beatroot) # for midiimport
add_subdirectory(../thirdparty/rtf2html rtf2html) # for capella

target_link_libraries(${MODULE}
    ${QT_LIBRARIES}
    audio       # for midiimport
    beatroot    # for midiimport
    rtf2html    # for capella
    )

if (NOT MSVC)
   set_target_properties (
      ${MODULE}
      PROPERTIES
         COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wextra -Winvalid-pch"
      )
else (NOT MSVC)
   set_target_properties (
      ${MODULE}
      PROPERTIES
         COMPILE_FLAGS "${PCH_INCLUDE}"
      )
endif (NOT MSVC)   

xcode_pch(${MODULE} all)

# Use MSVC pre-compiled headers
vstudio_pch( ${MODULE} )

# MSVC does not depend on mops1 & mops2 for PCH
if (NOT MSVC)
   ADD_DEPENDENCIES(${MODULE} mops1)
   ADD_DEPENDENCIES(${MODULE} mops2)
endif (NOT MSVC)   

