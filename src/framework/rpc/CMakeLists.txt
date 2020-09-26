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

set(MODULE rpc)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/rpctypes.h
    ${CMAKE_CURRENT_LIST_DIR}/irpcserver.h
    ${CMAKE_CURRENT_LIST_DIR}/irpcclient.h
    ${CMAKE_CURRENT_LIST_DIR}/rpcmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/rpcmodule.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/client.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/client.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/server.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/server.h
)

include(${PROJECT_SOURCE_DIR}/build/module.cmake)


