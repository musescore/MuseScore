#=============================================================================
#  MuseScore
#  Music Composition & Notation
#
#  Copyright (C) 2020 MuseScore BVBA and others
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

# This is module for convert mscz files to various formats (image, pdf, audio and etc).
# Main goal - determine what need to do and call necceasers functions.
# Usually use from command line.

set(MODULE converter)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/convertermodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/convertermodule.h
    ${CMAKE_CURRENT_LIST_DIR}/convertercodes.h
    ${CMAKE_CURRENT_LIST_DIR}/iconvertercontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/convertercontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/convertercontroller.h
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)

