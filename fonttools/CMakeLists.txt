#=============================================================================
#  MuseScore
#  Music Composition & Notation
#  $Id:$
#
#  Copyright (C) 2011 Werner Schweer
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2
#  as published by the Free Software Foundation and appearing in
#  the file LICENSE.GPL
#=============================================================================

set(CMAKE_VERBOSE_MAKEFILE OFF)

add_executable(
      genft
      genft.cpp
      )

target_link_libraries(genft ${QT_LIBRARIES} -lfreetype)

if (NOT MSVC)
   set_target_properties(genft PROPERTIES COMPILE_FLAGS "-I/usr/include/freetype2 -g -Wall -Wextra -Winvalid-pch")
else (NOT MSVC)
   set_target_properties (
      genft
      PROPERTIES
         COMPILE_FLAGS ""     # Might be enough, have to check if a forced directory or forced include is needed.
      )
endif (NOT MSVC)   

