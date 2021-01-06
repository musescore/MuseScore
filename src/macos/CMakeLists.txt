#=============================================================================
#  MuseScore
#  Music Composition & Notation
#
#  Copyright (C) 2021 MuseScore BVBA and others
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License version 2.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#=============================================================================

if(OS_IS_MAC)

    set(MODULE macos)

    set(MODULE_SRC
        ${CMAKE_CURRENT_LIST_DIR}/macosmodule.cpp
        ${CMAKE_CURRENT_LIST_DIR}/macosmodule.h
        ${CMAKE_CURRENT_LIST_DIR}/imacos.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/macos.mm
        ${CMAKE_CURRENT_LIST_DIR}/internal/macos.h
        )

    set(MODULE_USE_UNITY_NONE ON) # doesn't work due to Objective-C++ code
    # TODO: control that Cmake puts the C++ code and the Objective-C++ code in
    # separate unities (and compile one as Objective-C++), so that it will work.

    include(${PROJECT_SOURCE_DIR}/build/module.cmake)

endif()
