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

declare_module(muse_network)
set(MODULE_ALIAS muse::network)

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

if (MUSE_MODULE_NETWORK_WEBSOCKET)
    set(MODULE_SRC ${MODULE_SRC}
        ${CMAKE_CURRENT_LIST_DIR}/api/websocketapi.cpp
        ${CMAKE_CURRENT_LIST_DIR}/api/websocketapi.h
        ${CMAKE_CURRENT_LIST_DIR}/api/websocketserverapi.cpp
        ${CMAKE_CURRENT_LIST_DIR}/api/websocketserverapi.h
    )

    set(MODULE_DEF -DMUSE_MODULE_NETWORK_WEBSOCKET)
endif()

setup_module()

if (MUSE_MODULE_NETWORK_TESTS)
    add_subdirectory(tests)
endif()
