#=============================================================================
#  MusE
#  Linux Music Editor
#  $Id:$
#
#  Copyright (C) 2002-2006 by Werner Schweer and others
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

QT4_WRAP_CPP (mocs
      aslider.h
      knob.h
      panknob.h
      volknob.h
      midipanknob.h
      colorlabel.h
      slider.h
      volslider.h
      mslider.h
      pitchedit.h
      pitchlabel.h
      poslabel.h
      denomspinbox.h
      )

add_library (
      awl STATIC
      ${PROJECT_BINARY_DIR}/all.h
      ${PCH}
      ${mocs}
      aslider.cpp
      knob.cpp
      panknob.cpp
      volknob.cpp
      midipanknob.cpp
      colorlabel.cpp
      slider.cpp
      volslider.cpp
      mslider.cpp
      pitchedit.cpp
      pitchlabel.cpp
      poslabel.cpp
#      posedit.cpp
      utils.cpp
      denomspinbox.cpp
      )
set_target_properties (
      awl
      PROPERTIES
         COMPILE_FLAGS "-include ${PROJECT_BINARY_DIR}/all.h -g -Wall -Wextra -Winvalid-pch"
      )

ADD_DEPENDENCIES(awl mops1)
ADD_DEPENDENCIES(awl mops2)

if (NOT MINGW AND NOT APPLE)
QT4_WRAP_CPP (mocs2
      aslider.h
      knob.h
      panknob.h
      volknob.h
      midipanknob.h
      awlplugin.h
      colorlabel.h
      slider.h
      volslider.h
      mslider.h
      denomspinbox.h
      pitchedit.h
      pitchlabel.h
      awlplugin.h
      )

add_library (
      awlplugin SHARED
      ${PROJECT_BINARY_DIR}/all.h
      ${mocs2}
      awlplugin.cpp
      aslider.cpp
      knob.cpp
      panknob.cpp
      volknob.cpp
      midipanknob.cpp
      colorlabel.cpp
      slider.cpp
      volslider.cpp
      mslider.cpp
      denomspinbox.cpp
      pitchedit.cpp
      pitchlabel.cpp
      utils.cpp
      )

##  "-D_GNU_SOURCE -D_REENTRANT -DHAVE_CONFIG_H -DQT_PLUGIN -DQT_SHARED -DQT_NO_DEBUG -include ${PROJECT_SOURCE_DIR}/all.h"

#
# we cannot use our precompiled headers because of -fPIC
#
set_target_properties(awlplugin
      PROPERTIES
      COMPILE_FLAGS
         "-fPIC -D_GNU_SOURCE -D_REENTRANT -DHAVE_CONFIG_H -DQT_PLUGIN -DQT_SHARED -DQT_NO_DEBUG -include ${PROJECT_SOURCE_DIR}/all.h"
      )

install( TARGETS awlplugin DESTINATION ${QT_PLUGINS_DIR}/designer/ )

#add_executable(ltest ltest.cpp)
#target_link_libraries (ltest awlplugin msynth al ${QT_LIBRARIES})


endif (NOT MINGW AND NOT APPLE)
