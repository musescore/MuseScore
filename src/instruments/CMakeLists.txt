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

set(MODULE instruments)

set(MODULE_QRC instruments.qrc)

set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml )

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/instrumentsmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/instrumentsmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/instrumentstypes.h
    ${CMAKE_CURRENT_LIST_DIR}/iinstrumentsconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/iinstrumentsreader.h
    ${CMAKE_CURRENT_LIST_DIR}/iinstrumentsrepository.h
    ${CMAKE_CURRENT_LIST_DIR}/iselectinstrumentscenario.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/instrumentsreader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/instrumentsreader.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/instrumentsrepository.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/instrumentsrepository.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/instrumentsconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/instrumentsconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/selectinstrumentscenario.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/selectinstrumentscenario.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/instrumentsuiactions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/instrumentsuiactions.h
    ${CMAKE_CURRENT_LIST_DIR}/view/instrumentlistmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/instrumentlistmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/abstractinstrumentpaneltreeitem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/abstractinstrumentpaneltreeitem.h
    ${CMAKE_CURRENT_LIST_DIR}/view/roottreeitem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/roottreeitem.h
    ${CMAKE_CURRENT_LIST_DIR}/view/parttreeitem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/parttreeitem.h
    ${CMAKE_CURRENT_LIST_DIR}/view/instrumenttreeitem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/instrumenttreeitem.h
    ${CMAKE_CURRENT_LIST_DIR}/view/instrumentcontroltreeitem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/instrumentcontroltreeitem.h
    ${CMAKE_CURRENT_LIST_DIR}/view/stafftreeitem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/stafftreeitem.h
    ${CMAKE_CURRENT_LIST_DIR}/view/staffcontroltreeitem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/staffcontroltreeitem.h
    ${CMAKE_CURRENT_LIST_DIR}/view/instrumentpaneltreemodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/instrumentpaneltreemodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/staffsettingsmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/staffsettingsmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/instrumentsettingsmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/instrumentsettingsmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/instrumentpaneltreemodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/instrumentpaneltreemodel.h
    )

set(MODULE_LINK
    libmscore
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)
