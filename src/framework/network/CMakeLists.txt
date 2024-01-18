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

include(SetupModule)

if (MUE_BUILD_NETWORK_TESTS)
    add_subdirectory(tests)
endif()
