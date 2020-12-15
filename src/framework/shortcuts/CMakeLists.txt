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

set(MODULE shortcuts)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/shortcutsmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/shortcutsmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/shortcutstypes.h
    ${CMAKE_CURRENT_LIST_DIR}/ishortcutsregister.h
    ${CMAKE_CURRENT_LIST_DIR}/ishortcutscontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/ishortcutsconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/ishortcutcontextresolver.h
    ${CMAKE_CURRENT_LIST_DIR}/imidiremote.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/shortcutsinstancemodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/shortcutsinstancemodel.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/shortcutsregister.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/shortcutsregister.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/shortcutscontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/shortcutscontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/shortcutsconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/shortcutsconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/midiremote.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/midiremote.h
    )

set(MODULE_QRC shortcuts.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)

