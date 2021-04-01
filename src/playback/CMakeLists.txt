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

set(MODULE playback)

set(MODULE_QRC
    ${CMAKE_CURRENT_LIST_DIR}/playback.qrc
    )

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/playbackmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/playbackmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/iplaybackcontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/iplaybackconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/playbacktypes.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/playbackcontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/playbackcontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/playbackuiactions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/playbackuiactions.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/playbackconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/playbackconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/view/playbacktoolbarmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/playbacktoolbarmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/playbacksettingsmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/internal/playbacksettingsmodel.h
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)
