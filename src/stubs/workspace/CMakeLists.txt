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

set(MODULE workspace)
set(MODULE_QRC workspace.qrc)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/workspacestubmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/workspacestubmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/workspacestub.cpp
    ${CMAKE_CURRENT_LIST_DIR}/workspacestub.h
    ${CMAKE_CURRENT_LIST_DIR}/workspacemanagerstub.cpp
    ${CMAKE_CURRENT_LIST_DIR}/workspacemanagerstub.h
    ${CMAKE_CURRENT_LIST_DIR}/workspaceconfigurationstub.cpp
    ${CMAKE_CURRENT_LIST_DIR}/workspaceconfigurationstub.h
    ${CMAKE_CURRENT_LIST_DIR}/workspacedatastreamregisterstub.cpp
    ${CMAKE_CURRENT_LIST_DIR}/workspacedatastreamregisterstub.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspacecreatorstub.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspacecreatorstub.h
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)

