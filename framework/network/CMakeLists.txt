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

set(MODULE network)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/networkmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/networkmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/networkerrors.h
    ${CMAKE_CURRENT_LIST_DIR}/networktypes.h
    ${CMAKE_CURRENT_LIST_DIR}/inetworkmanager.h
    ${CMAKE_CURRENT_LIST_DIR}/inetworkmanagercreator.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/networkmanager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/networkmanager.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/networkmanagercreator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/networkmanagercreator.h
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)
