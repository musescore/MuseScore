#=============================================================================
#  MusE
#  Linux Music Editor
#
#  Copyright (C) 2002-2016 by Werner Schweer and others
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

add_library (awl STATIC
      aslider.cpp
      knob.cpp
      panknob.cpp
      volknob.cpp
      midipanknob.cpp
      colorlabel.cpp
      slider.cpp
      styledslider.cpp
      volslider.cpp
      mslider.cpp
      pitchedit.cpp
      pitchlabel.cpp
      poslabel.cpp
      utils.cpp
      denomspinbox.cpp
      aslider.h colorlabel.h denomspinbox.h fastlog.h
      knob.h midipanknob.h mslider.h panknob.h pitchedit.h pitchlabel.h
      poslabel.h slider.h styledslider.h
      utils.h volknob.h volslider.h
      )

add_library (awlplugin SHARED
      awlplugin.cpp
      aslider.cpp
      knob.cpp
      panknob.cpp
      volknob.cpp
      midipanknob.cpp
      colorlabel.cpp
      slider.cpp
      styledslider.cpp
      volslider.cpp
      mslider.cpp
      denomspinbox.cpp
      pitchedit.cpp
      pitchlabel.cpp
      utils.cpp
      )

# Don't install awlplugin on default
#install( TARGETS awlplugin DESTINATION ${designerPlugins}/plugins/designer/ )

#add_executable(ltest ltest.cpp)
#target_link_libraries (ltest awlplugin msynth al ${QT_LIBRARIES})

set_target_properties(awlplugin PROPERTIES EXCLUDE_FROM_ALL 1)

if (BUILD_PCH)
    target_use_pch(awl)
    target_use_pch(awlplugin)
endif (BUILD_PCH)
