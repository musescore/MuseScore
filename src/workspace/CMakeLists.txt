# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore BVBA and others
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

set(MODULE workspace)
set(MODULE_QRC workspace.qrc)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/workspacemodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/workspacemodule.h
    ${CMAKE_CURRENT_LIST_DIR}/iworkspace.h
    ${CMAKE_CURRENT_LIST_DIR}/workspacetypes.h
    ${CMAKE_CURRENT_LIST_DIR}/iworkspacemanager.h
    ${CMAKE_CURRENT_LIST_DIR}/iworkspaceconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/iworkspacedatastream.h
    ${CMAKE_CURRENT_LIST_DIR}/iworkspacedatastreamregister.h
    ${CMAKE_CURRENT_LIST_DIR}/view/workspacelistmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/workspacelistmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/currentworkspacemodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/currentworkspacemodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/newworkspacemodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/newworkspacemodel.h
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
    ${CMAKE_CURRENT_LIST_DIR}/internal/iworkspacecreator.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspacecreator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspacecreator.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspacesettings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspacesettings.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspaceuiactions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspaceuiactions.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspaceactioncontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspaceactioncontroller.h
    )

if (NOT MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 9.0)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-copy")
endif (NOT MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 9.0)

set(MODULE_LINK
    qzip
    engraving
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)

