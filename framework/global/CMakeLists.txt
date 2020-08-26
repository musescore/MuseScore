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

set(MODULE global)

include(${CMAKE_CURRENT_LIST_DIR}/modularity/modularity.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/async/async.cmake)

set(MODULE_SRC
    ${MODULARITY_SRC}
    ${ASYNC_SRC}
    ${CMAKE_CURRENT_LIST_DIR}/globalmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/globalmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/iinteractive.h
    ${CMAKE_CURRENT_LIST_DIR}/iglobalconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/io/path.cpp
    ${CMAKE_CURRENT_LIST_DIR}/io/path.h
    ${CMAKE_CURRENT_LIST_DIR}/log.h
    ${CMAKE_CURRENT_LIST_DIR}/dataformatter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dataformatter.h
    ${CMAKE_CURRENT_LIST_DIR}/val.cpp
    ${CMAKE_CURRENT_LIST_DIR}/val.h
    ${CMAKE_CURRENT_LIST_DIR}/settings.cpp
    ${CMAKE_CURRENT_LIST_DIR}/settings.h
    ${CMAKE_CURRENT_LIST_DIR}/version.cpp
    ${CMAKE_CURRENT_LIST_DIR}/version.h
    ${CMAKE_CURRENT_LIST_DIR}/stringutils.cpp
    ${CMAKE_CURRENT_LIST_DIR}/stringutils.h
    ${CMAKE_CURRENT_LIST_DIR}/ptrutils.h
    ${CMAKE_CURRENT_LIST_DIR}/realfn.h
    ${CMAKE_CURRENT_LIST_DIR}/runtime.cpp
    ${CMAKE_CURRENT_LIST_DIR}/runtime.h
    ${CMAKE_CURRENT_LIST_DIR}/translation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/translation.h
    ${CMAKE_CURRENT_LIST_DIR}/ret.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ret.h
    ${CMAKE_CURRENT_LIST_DIR}/retval.h
    ${CMAKE_CURRENT_LIST_DIR}/uri.cpp
    ${CMAKE_CURRENT_LIST_DIR}/uri.h
    ${CMAKE_CURRENT_LIST_DIR}/invoker.cpp
    ${CMAKE_CURRENT_LIST_DIR}/invoker.h
    ${CMAKE_CURRENT_LIST_DIR}/smuflranges.cpp
    ${CMAKE_CURRENT_LIST_DIR}/smuflranges.h
    ${CMAKE_CURRENT_LIST_DIR}/widgetstatestore.cpp
    ${CMAKE_CURRENT_LIST_DIR}/widgetstatestore.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/globalconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/globalconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/interactive.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/interactive.h
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)
