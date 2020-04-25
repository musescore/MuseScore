#=============================================================================
#  MuseScore
#  Music Composition & Notation
#
#  Copyright (C) 2013 Werner Schweer
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2
#  as published by the Free Software Foundation and appearing in
#  the file LICENSE.GPL
#=============================================================================

set(MODULE audio)

include (${PROJECT_SOURCE_DIR}/build/gch.cmake)

if (APPLE)
      file(GLOB_RECURSE INCS "*.h")
else (APPLE)
      set(INCS "")
endif (APPLE)

if (NOT MSVC)
   set(_all_h_file "${PROJECT_BINARY_DIR}/all.h")
else (NOT MSVC)
   set(_all_h_file "${PROJECT_SOURCE_DIR}/all.h")
endif (NOT MSVC)

include_directories (
      ${PROJECT_SOURCE_DIR}
      ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_LIST_DIR}
      ${PROJECT_SOURCE_DIR}/framework/global
      ${PROJECT_SOURCE_DIR}/thirdparty  # because include "mscore/musescore.h"
      )

include (${CMAKE_CURRENT_LIST_DIR}/midi/midi.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/drivers/drivers.cmake)
include (${CMAKE_CURRENT_LIST_DIR}/exports/exports.cmake)

QT5_WRAP_UI (ui_headers
    ${MIDI_UI}
    )

add_library (${MODULE} STATIC
      ${_all_h_file}
      ${PCH}
      ${ui_headers}
      ${MIDI_SRC}
      ${DRIVERS_SRC}
      ${EXPORTS_SRC}
      )

target_link_libraries (${MODULE}
      ${QT_LIBRARIES}
      audiofile
      mscoreapp
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


