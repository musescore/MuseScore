# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2026 MuseScore Limited
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
# along with this program. If not, see <https://www.gnu.org/licenses/>.

muse_create_module(muse_network ALIAS muse::network)

target_sources(muse_network PRIVATE
    networkmodule.cpp
    networkmodule.h
    networkerrors.h
    networktypes.h
    inetworkmanager.h
    inetworkmanagercreator.h
    inetworkconfiguration.h

    internal/networkmanager.cpp
    internal/networkmanager.h
    internal/networkmanagercreator.cpp
    internal/networkmanagercreator.h
    internal/networkconfiguration.cpp
    internal/networkconfiguration.h
)

target_link_libraries(muse_network PUBLIC Qt::Network)

if (MUSE_MODULE_NETWORK_WEBSOCKET)
    target_sources(muse_network PRIVATE
        api/websocketapi.cpp
        api/websocketapi.h
        api/websocketserverapi.cpp
        api/websocketserverapi.h
    )

    target_compile_definitions(muse_network PRIVATE MUSE_MODULE_NETWORK_WEBSOCKET)
    target_link_libraries(muse_network PRIVATE Qt::WebSockets)
endif()

if (MUSE_MODULE_NETWORK_TESTS)
    add_subdirectory(tests)
endif()
