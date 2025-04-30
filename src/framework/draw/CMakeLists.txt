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

declare_module(muse_draw)
set(MODULE_ALIAS muse::draw)

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
    ${CMAKE_CURRENT_LIST_DIR}/types/drawdata.h
    ${CMAKE_CURRENT_LIST_DIR}/types/fontstypes.h

    ${CMAKE_CURRENT_LIST_DIR}/painter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/painter.h
    ${CMAKE_CURRENT_LIST_DIR}/ipaintprovider.h
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
    ${CMAKE_CURRENT_LIST_DIR}/utils/drawdatajson.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utils/drawdatajson.h
    ${CMAKE_CURRENT_LIST_DIR}/utils/drawdatacomp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utils/drawdatacomp.h
    ${CMAKE_CURRENT_LIST_DIR}/utils/drawdatarw.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utils/drawdatarw.h
    ${CMAKE_CURRENT_LIST_DIR}/utils/drawdatapaint.cpp
    ${CMAKE_CURRENT_LIST_DIR}/utils/drawdatapaint.h
    )

if (DRAW_NO_INTERNAL)
    set(MODULE_DEF -DDRAW_NO_INTERNAL -DDRAW_NO_QSVGRENDER)
else()
    set(MODULE_SRC ${MODULE_SRC}
        ${CMAKE_CURRENT_LIST_DIR}/internal/qpainterprovider.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/qpainterprovider.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/qimageprovider.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/qimageprovider.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/qimagepainterprovider.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/qimagepainterprovider.h
        )

    # fonts
    set(MODULE_SRC ${MODULE_SRC}
        ${CMAKE_CURRENT_LIST_DIR}/internal/fontproviderdispatcher.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/fontproviderdispatcher.h

        ${CMAKE_CURRENT_LIST_DIR}/internal/qfontprovider.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/qfontprovider.h
        #${CMAKE_CURRENT_LIST_DIR}/internal/fontengineft.cpp
        #${CMAKE_CURRENT_LIST_DIR}/internal/fontengineft.h

        ${CMAKE_CURRENT_LIST_DIR}/internal/ifontsengine.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/ifontsdatabase.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/ifontface.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/fontprovider.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/fontprovider.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/fontsdatabase.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/fontsdatabase.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/fontsengine.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/fontsengine.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/fontfaceft.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/fontfaceft.h
        ${CMAKE_CURRENT_LIST_DIR}/internal/fontfacedu.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/fontfacedu.h
    )

    include(cmake/SetupFreeType.cmake)
    include(cmake/SetupHarfBuzz.cmake)

    set(MODULE_DEF ${MODULE_DEF} -DMUSE_MODULE_DRAW_USE_QTTEXTDRAW)

    set(MODULE_LINK ${FREETYPE_LIBRARIES} ${HARFBUZZ_LIBRARIES} )#msdfgen)

    set(MODULE_INCLUDE_PRIVATE ${FREETYPE_INCLUDE_DIRS} ${HARFBUZZ_INCLUDE_DIRS} )#${MSDFGEN_INCLUDE_DIRS})

endif()

if (MUSE_MODULE_DRAW_TRACE)
    set(MODULE_DEF ${MODULE_DEF} -DMUSE_MODULE_DRAW_TRACE)
endif()

if (QT_SUPPORT)
    list(APPEND MODULE_LINK Qt::Gui Qt::Svg)
endif()

setup_module()

if (MUSE_MODULE_DRAW_TESTS)
    add_subdirectory(tests)
endif()
