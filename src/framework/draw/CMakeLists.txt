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

set(MODULE draw)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/drawmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/drawmodule.h

    ${CMAKE_CURRENT_LIST_DIR}/types/color.cpp
    ${CMAKE_CURRENT_LIST_DIR}/types/color.h
    ${CMAKE_CURRENT_LIST_DIR}/types/rgba.h
    ${CMAKE_CURRENT_LIST_DIR}/types/geometry.h
    ${CMAKE_CURRENT_LIST_DIR}/types/transform.h
    ${CMAKE_CURRENT_LIST_DIR}/types/transform.cpp
    ${CMAKE_CURRENT_LIST_DIR}/types/matrix.h
    ${CMAKE_CURRENT_LIST_DIR}/types/painterpath.h
    ${CMAKE_CURRENT_LIST_DIR}/types/painterpath.cpp
    ${CMAKE_CURRENT_LIST_DIR}/types/bezier.h
    ${CMAKE_CURRENT_LIST_DIR}/types/bezier.cpp
    ${CMAKE_CURRENT_LIST_DIR}/types/drawtypes.h
    ${CMAKE_CURRENT_LIST_DIR}/types/pen.h
    ${CMAKE_CURRENT_LIST_DIR}/types/brush.h
    ${CMAKE_CURRENT_LIST_DIR}/types/pixmap.h
    ${CMAKE_CURRENT_LIST_DIR}/types/font.cpp
    ${CMAKE_CURRENT_LIST_DIR}/types/font.h

    ${CMAKE_CURRENT_LIST_DIR}/painter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/painter.h
    ${CMAKE_CURRENT_LIST_DIR}/ipaintprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/buffereddrawtypes.h
    ${CMAKE_CURRENT_LIST_DIR}/bufferedpaintprovider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/bufferedpaintprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/svgrenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/svgrenderer.h
    ${CMAKE_CURRENT_LIST_DIR}/ifontprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/iimageprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/fontmetrics.cpp
    ${CMAKE_CURRENT_LIST_DIR}/fontmetrics.h

    ${CMAKE_CURRENT_LIST_DIR}/utils/drawlogger.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utils/drawlogger.h
    ${CMAKE_CURRENT_LIST_DIR}/utils/drawjson.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utils/drawjson.h
    ${CMAKE_CURRENT_LIST_DIR}/utils/drawcomp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utils/drawcomp.h
    )

if (DRAW_NO_INTERNAL)
    set(MODULE_DEF -DDRAW_NO_INTERNAL)
else()
    set(MODULE_SRC ${MODULE_SRC}
        ${CMAKE_CURRENT_LIST_DIR}/internal/qpainterprovider.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/qpainterprovider.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/qimageprovider.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/qimageprovider.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/qfontprovider.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/qfontprovider.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/fontengineft.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/fontengineft.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/qimagepainterprovider.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/qimagepainterprovider.h
        )

    if (USE_SYSTEM_FREETYPE)
         set(MODULE_LINK freetype)
    else (USE_SYSTEM_FREETYPE)
         set(MODULE_LINK mscore_freetype)
    endif (USE_SYSTEM_FREETYPE)
endif()

include(SetupModule)
