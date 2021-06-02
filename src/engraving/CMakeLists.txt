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

set(MODULE engraving)

include(${CMAKE_CURRENT_LIST_DIR}/libmscore/libmscore.cmake)

set(MODULE_SRC

    ${CMAKE_CURRENT_LIST_DIR}/engravingmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/engravingmodule.h

    ${LIBMSCORE_SRC}

    ${CMAKE_CURRENT_LIST_DIR}/draw/drawtypes.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/painter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/painter.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/ipaintprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/qpainterprovider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/qpainterprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/buffereddrawtypes.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/bufferedpaintprovider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/bufferedpaintprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/ifontprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/font.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/font.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/fontmetrics.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/fontmetrics.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/fontcompat.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/fontcompat.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/qfontprovider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/qfontprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/fontengineft.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/fontengineft.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/utils/drawlogger.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/utils/drawlogger.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/utils/drawjson.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/utils/drawjson.h
    ${CMAKE_CURRENT_LIST_DIR}/draw/utils/drawcomp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/draw/utils/drawcomp.h

    ${CMAKE_CURRENT_LIST_DIR}/accessibility/accessiblescore.cpp
    ${CMAKE_CURRENT_LIST_DIR}/accessibility/accessiblescore.h
    ${CMAKE_CURRENT_LIST_DIR}/accessibility/accessibleelement.cpp
    ${CMAKE_CURRENT_LIST_DIR}/accessibility/accessibleelement.h
    ${CMAKE_CURRENT_LIST_DIR}/accessibility/accessiblenote.cpp
    ${CMAKE_CURRENT_LIST_DIR}/accessibility/accessiblenote.h
    )

set(MODULE_INCLUDE
    ${PROJECT_SOURCE_DIR}/thirdparty/dtl
    )

set(FREETYPE_LIB )
if (USE_SYSTEM_FREETYPE)
     set(MODULE_LINK freetype)
else (USE_SYSTEM_FREETYPE)
     set(MODULE_LINK mscore_freetype)
endif (USE_SYSTEM_FREETYPE)

set(MODULE_LINK ${MODULE_LINK} midi_old qzip)

set(MODULE_USE_UNITY_NONE ON) # not work
include(${PROJECT_SOURCE_DIR}/build/module.cmake)

set_property(TARGET ${MODULE} APPEND PROPERTY AUTOMOC_MACRO_NAMES "BEGIN_QT_REGISTERED_ENUM")
