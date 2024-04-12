# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited
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

declare_module(workspacescene)
set(MODULE_QRC workspacescene.qrc)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/workspacescenemodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/workspacescenemodule.h

    ${CMAKE_CURRENT_LIST_DIR}/view/workspacelistmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/workspacelistmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/newworkspacemodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/newworkspacemodel.h
    )

if (NOT MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 9.0)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-copy")
endif (NOT MSVC AND CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 9.0)

setup_module()

