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
    ${CMAKE_CURRENT_LIST_DIR}/ipaletteadapter.h
    ${CMAKE_CURRENT_LIST_DIR}/ipaletteconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/palettetypes.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/palette_config.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/mu4paletteadapter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/mu4paletteadapter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspacepalettestream.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/workspacepalettestream.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/paletteconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/paletteconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/paletteworkspacesetup.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/paletteworkspacesetup.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/paletteuiactions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/paletteuiactions.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/paletteactionscontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/paletteactionscontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/view/paletterootmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/paletterootmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/palettepropertiesmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/palettepropertiesmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/palettecellpropertiesmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/palettecellpropertiesmodel.cpp
    )

set(MODULE_UI
    ${PALETTE_UI}
    ${WIDGETS_UI}
    )

set(MODULE_INCLUDE
    ${CMAKE_CURRENT_LIST_DIR}/internal
    ${CMAKE_CURRENT_LIST_DIR}/internal/widgets
    )

set(MODULE_LINK
    qzip
    libmscore
    commonscene
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)
