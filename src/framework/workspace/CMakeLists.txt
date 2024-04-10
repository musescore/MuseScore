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

declare_module(muse_workspace)
set(MODULE_ALIAS muse::workspace)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/workspacemodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/workspacemodule.h
    ${CMAKE_CURRENT_LIST_DIR}/workspacetypes.h
    ${CMAKE_CURRENT_LIST_DIR}/workspaceerrors.h
    ${CMAKE_CURRENT_LIST_DIR}/iworkspaceconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/iworkspace.h
    ${CMAKE_CURRENT_LIST_DIR}/iworkspacemanager.h
    ${CMAKE_CURRENT_LIST_DIR}/iworkspacesdataprovider.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/workspaceconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspaceconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspace.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspace.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspacemanager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspacemanager.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspacesdataprovider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspacesdataprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspacefile.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspacefile.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspaceuiactions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspaceuiactions.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspaceactioncontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspaceactioncontroller.h
    )

if (NOT MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 9.0)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-copy")
endif (NOT MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 9.0)

setup_module()

