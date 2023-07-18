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
    engraving
    )

include(SetupModule)

if (MUE_BUILD_UNIT_TESTS)
    add_subdirectory(tests)
endif()
