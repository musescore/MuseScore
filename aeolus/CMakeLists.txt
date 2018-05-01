#=============================================================================
#  Mscore
#  Linux Music Score Editor
#  $Id:$
#
#  Copyright (C) 2010 by Werner Schweer and others
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

if (APPLE)
        file(GLOB_RECURSE INCS "*.h")
else (APPLE)
        set(INCS "")
endif (APPLE)


QT5_WRAP_UI (aeolusUi aeolus_gui.ui)

add_library (aeolus STATIC
      ${aeolusUi}
      aeolus.cpp audio.cpp model.cpp addsynth.cpp scales.cpp
      asection.cpp division.cpp rankwave.cpp
      rngen.cpp exp2ap.cpp
      aeolusgui.cpp sparm.cpp
      ${PROJECT_BINARY_DIR}/all.h
      ${PCH}
      ${INCS}
      )

if (NOT MSVC)
   set_target_properties (
      aeolus
      PROPERTIES
         COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wextra -Winvalid-pch"
      )
else (NOT MSVC)
   set_target_properties (
      aeolus
      PROPERTIES
         COMPILE_FLAGS "${PCH_INCLUDE} ${PCH_FORCE_USE}"
      )
endif (NOT MSVC)   

install(DIRECTORY
      stops
      DESTINATION ${Mscore_SHARE_NAME}${Mscore_INSTALL_NAME}sound/aeolus
         PATTERN .svn EXCLUDE
      )

xcode_pch(aeolus all)

ADD_DEPENDENCIES(aeolus mops1)
ADD_DEPENDENCIES(aeolus mops2)

