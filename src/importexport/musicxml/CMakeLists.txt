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

set(MODULE iex_musicxml)

set(MODULE_QRC musicxml.qrc)

include(${CMAKE_CURRENT_LIST_DIR}/internal/musicxml/musicxml.cmake)

set(MODULE_SRC
    ${MUSICXML_SRC}
    ${CMAKE_CURRENT_LIST_DIR}/musicxmlmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/musicxmlmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/imusicxmlconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/musicxmlconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/musicxmlconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/musicxmlreader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/musicxmlreader.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/musicxmlwriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/musicxmlwriter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/mxlwriter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/mxlwriter.h
    )

set(MODULE_LINK
    libmscore
    qzip
    notation
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)

