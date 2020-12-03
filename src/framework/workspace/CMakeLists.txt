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

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/workspacemodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/workspacemodule.h
    ${CMAKE_CURRENT_LIST_DIR}/iworkspace.h
    ${CMAKE_CURRENT_LIST_DIR}/workspacetypes.h
    ${CMAKE_CURRENT_LIST_DIR}/iworkspacemanager.h
    ${CMAKE_CURRENT_LIST_DIR}/iworkspaceconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/iworkspacedatastream.h
    ${CMAKE_CURRENT_LIST_DIR}/iworkspacedatastreamregister.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspaceconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspaceconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspace.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspace.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspacemanager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspacemanager.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspacefile.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspacefile.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspacedatastreamregister.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspacedatastreamregister.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspacesettingsstream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspacesettingsstream.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspacetoolbarstream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspacetoolbarstream.h
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)

