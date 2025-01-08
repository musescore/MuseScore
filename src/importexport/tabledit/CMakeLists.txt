# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2025 MuseScore Limited
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

declare_module(iex_tabledit)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/tableditmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tableditmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/tableditreader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/tableditreader.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/importtef.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/importtef.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/tuplethandler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/tuplethandler.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/voiceallocator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/voiceallocator.h
    )

setup_module()

if (MUE_BUILD_IMPORTEXPORT_TESTS)
    add_subdirectory(tests)
endif()
