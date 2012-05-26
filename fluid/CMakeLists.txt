#=============================================================================
#  Mscore
#  Linux Music Score Editor
#  $Id:$
#
#  Copyright (C) 2007 by Werner Schweer and others
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

set(SRC
  dsp.cpp fluid.cpp voice.cpp chan.cpp sfont.cpp chorus.cpp
  conv.cpp gen.cpp mod.cpp rev.cpp tuning.cpp
  )

if (SOUNDFONT3)
      set(SRC ${SRC} sfont3.cpp)
endif (SOUNDFONT3)

add_library (fluid STATIC
      ${PROJECT_BINARY_DIR}/all.h
      ${PCH}
      ${SRC}
      )
set_target_properties (
      fluid
      PROPERTIES
         COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h -g -Wall -Wextra -Winvalid-pch"
      )

ADD_DEPENDENCIES(fluid mops1)
ADD_DEPENDENCIES(fluid mops2)

