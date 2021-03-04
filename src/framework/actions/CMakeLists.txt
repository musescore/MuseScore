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

set(MODULE actions)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/actionsmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/actionsmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/iactionsdispatcher.h
    ${CMAKE_CURRENT_LIST_DIR}/imoduleactions.h
    ${CMAKE_CURRENT_LIST_DIR}/iactionsregister.h
    ${CMAKE_CURRENT_LIST_DIR}/iactionsavailability.h
    ${CMAKE_CURRENT_LIST_DIR}/actionable.h
    ${CMAKE_CURRENT_LIST_DIR}/actiontypes.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/actionsdispatcher.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/actionsdispatcher.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/actionsregister.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/actionsregister.h
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)
