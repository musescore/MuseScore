#=============================================================================
#  MuseScore
#  Music Composition & Notation
#  $Id:$
#
#  Copyright (C) 2014 Werner Schweer
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

add_library(xmlstream STATIC
   xmlstream.cpp xmlutils.cpp
   ${PROJECT_BINARY_DIR}/all.h
   ${PCH}
   ${INCS}
   )

set_target_properties (
      xmlstream
      PROPERTIES
         COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wextra -Winvalid-pch"
      )

xcode_pch(xmlstream all)

ADD_DEPENDENCIES(xmlstream mops1)
ADD_DEPENDENCIES(xmlstream mops2)

