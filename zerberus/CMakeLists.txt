#=============================================================================
#  MuseScore
#  Music Composition & Notation
#
#  Copyright (C) 2013 Werner Schweer
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2
#  as published by the Free Software Foundation and appearing in
#  the file LICENCE.GPL
#=============================================================================

include (${PROJECT_SOURCE_DIR}/build/gch.cmake)

QT5_WRAP_UI (zerberusUi zerberus_gui.ui)

add_library (zerberus STATIC
      ${zerberusUi}
      channel.cpp
      filter.cpp
      instrument.cpp
      sfz.cpp
      voice.cpp
      zerberus.cpp
      zone.cpp
      zerberusgui.cpp
      ${PROJECT_BINARY_DIR}/all.h
      ${PCH}
      ${INCS}
      )

set_target_properties (
      zerberus
      PROPERTIES
         COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wextra -Winvalid-pch"
      )

xcode_pch(zerberus all)

ADD_DEPENDENCIES(zerberus mops1)
ADD_DEPENDENCIES(zerberus mops2)

