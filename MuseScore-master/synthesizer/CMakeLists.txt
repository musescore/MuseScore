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

add_library (synthesizer STATIC
      ${PROJECT_BINARY_DIR}/all.h
      ${PCH}
      msynthesizer.cpp
      event.cpp
      synthesizergui.cpp
      ${INCS}
      )
set_target_properties (
      synthesizer
      PROPERTIES
         COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wextra -Winvalid-pch"
      )

xcode_pch(synthesizer all)

ADD_DEPENDENCIES(synthesizer mops1)
ADD_DEPENDENCIES(synthesizer mops2)

