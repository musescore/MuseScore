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

set(MODULE palette)

set(MODULE_QRC
    ${CMAKE_CURRENT_LIST_DIR}/palette.qrc
    )

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml)

include(${CMAKE_CURRENT_LIST_DIR}/internal/palette/palette.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/internal/widgets/widgets.cmake)

set(MODULE_SRC
    ${PALETTE_SRC}
    ${WIDGETS_SRC}
    ${CMAKE_CURRENT_LIST_DIR}/palettemodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/palettemodule.h
    )

set(MODULE_UI
    ${PALETTE_UI}
    ${WIDGETS_UI}
    )

set(MODULE_INCLUDE
    ${PROJECT_SOURCE_DIR}/mscore
    ${PROJECT_SOURCE_DIR}/thirdparty
    ${PROJECT_SOURCE_DIR}/mu4/scenes/palette/internal
    ${PROJECT_SOURCE_DIR}/mu4/scenes/palette/internal/widgets
    ${PROJECT_BINARY_DIR}/mscore
    )

set(MODULE_LINK
    libmscore
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)
