#=============================================================================
#  MusE
#  Linux Music Editor
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

add_library (
      mstyle STATIC
      ${PROJECT_BINARY_DIR}/all.h
      ${PCH}
      ${INCS}
      mstyle.cpp colorscheme.cpp colorutils.cpp tileset.cpp stylehelper.cpp frameshadow.cpp
      animations.cpp animationdata.cpp dockseparatordata.cpp
      dockseparatorengine.cpp enabledata.cpp genericdata.cpp headerviewdata.cpp
      headerviewengine.cpp mdiwindowdata.cpp mdiwindowengine.cpp menubardata.cpp
      menubarengine.cpp menuengine.cpp progressbardata.cpp
      progressbarengine.cpp scrollbardata.cpp scrollbarengine.cpp sliderdata.cpp
      sliderengine.cpp spinboxdata.cpp spinboxengine.cpp splitterengine.cpp
      tabbardata.cpp tabbarengine.cpp toolbardata.cpp toolbarengine.cpp
      toolboxengine.cpp widgetstatedata.cpp widgetstateengine.cpp mconfig.cpp
      comboboxdata.cpp comboboxengine.cpp labeldata.cpp labelengine.cpp lineeditdata.cpp
      lineeditengine.cpp stackedwidgetdata.cpp stackedwidgetengine.cpp transitiondata.cpp
      transitions.cpp transitionwidget.cpp
      baseengine.h menudata.h animation.h
      )

set_target_properties (
      mstyle
      PROPERTIES
         COMPILE_FLAGS "${PCH_INCLUDE} -g -Wall -Wextra -Winvalid-pch"
      )

xcode_pch(mstyle all)

ADD_DEPENDENCIES(mstyle mops1)
ADD_DEPENDENCIES(mstyle mops2)

