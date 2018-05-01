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

include (${PROJECT_SOURCE_DIR}/build/gch.cmake)

if (APPLE)
      file(GLOB_RECURSE INCS "*.h")
else (APPLE)
      set(INCS "")
endif (APPLE)

QT5_ADD_RESOURCES (qrc_effects_files
     zita1/zita.qrc
     )

QT5_WRAP_UI (ui_headers
      compressor/compressor_gui.ui
      )

if (NOT MSVC)
   set(_all_h_file "${PROJECT_BINARY_DIR}/all.h")
else (NOT MSVC)
   set(_all_h_file "${PROJECT_SOURCE_DIR}/all.h")
endif (NOT MSVC)

add_library (effects STATIC
      ${ui_headers}
      ${qrc_effects_files}
      ${_all_h_file}
      ${PCH}
      effect.cpp
      effectgui.cpp
      noeffect/noeffect.cpp
      noeffect/noeffectgui.h
      zita1/zita.cpp
      zita1/zitagui.cpp
      compressor/compressor.cpp
      compressor/compressorgui.cpp
      ${INCS}
      )

if (NOT MSVC)
   set_target_properties (
      effects
      PROPERTIES
         COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wextra -Winvalid-pch"
      )
else (NOT MSVC)
   set_target_properties (
      effects
      PROPERTIES
         COMPILE_FLAGS "${PCH_INCLUDE}"
      )
endif (NOT MSVC)   

xcode_pch(effects all)

# Use MSVC pre-compiled headers
vstudio_pch( effects )

# MSVC does not depend on mops1 & mops2 for PCH
if (NOT MSVC)
   ADD_DEPENDENCIES(effects mops1)
   ADD_DEPENDENCIES(effects mops2)
endif (NOT MSVC)   

