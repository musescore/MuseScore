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

QT4_ADD_RESOURCES (qrc_effects_files
      zita1/zita.qrc
      )

add_library (effects STATIC
      ${qrc_effects_files}
      ${PROJECT_BINARY_DIR}/all.h
      ${PCH}
      effect.cpp
      effectgui.cpp
      noeffect/noeffect.cpp
      noeffect/noeffectgui.h
      zita1/zita.cpp
      zita1/zitagui.cpp
      ${INCS}
      )
set_target_properties (
      effects
      PROPERTIES
         COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wextra -Winvalid-pch"
      )

xcode_pch(effects all)

ADD_DEPENDENCIES(effects mops1)
ADD_DEPENDENCIES(effects mops2)

