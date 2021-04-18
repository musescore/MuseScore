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

set(MODULE iex_guitarpro)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/guitarpromodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/guitarpromodule.h
    ${CMAKE_CURRENT_LIST_DIR}/iguitarproconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/guitarproconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/guitarproconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/guitarproreader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/guitarproreader.h

     ${CMAKE_CURRENT_LIST_DIR}/internal/importgtp.cpp
     ${CMAKE_CURRENT_LIST_DIR}/internal/importgtp-gp4.cpp
     ${CMAKE_CURRENT_LIST_DIR}/internal/importgtp-gp5.cpp
     ${CMAKE_CURRENT_LIST_DIR}/internal/importgtp-gp6.cpp
     ${CMAKE_CURRENT_LIST_DIR}/internal/importgtp-gp7.cpp
     ${CMAKE_CURRENT_LIST_DIR}/internal/importgtp.h
     ${CMAKE_CURRENT_LIST_DIR}/internal/importptb.cpp
     ${CMAKE_CURRENT_LIST_DIR}/internal/importptb.h
    )

set(MODULE_LINK
    libmscore
    qzip
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)

