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

set(MODULE ui)

set(MODULE_QRC ui.qrc)
set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/uimodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/uimodule.h
    ${CMAKE_CURRENT_LIST_DIR}/iuiengine.h
    ${CMAKE_CURRENT_LIST_DIR}/iinteractiveprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/iinteractiveuriregister.h
    ${CMAKE_CURRENT_LIST_DIR}/iuiconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/uierrors.h
    ${CMAKE_CURRENT_LIST_DIR}/uitypes.h
    ${CMAKE_CURRENT_LIST_DIR}/imainwindow.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/interactiveuriregister.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/interactiveuriregister.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/uiengine.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/uiengine.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/uiconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/uiconfiguration.h

    ${CMAKE_CURRENT_LIST_DIR}/view/interactiveprovider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/interactiveprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/view/qmlapi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/qmlapi.h
    ${CMAKE_CURRENT_LIST_DIR}/view/qmltheme.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/qmltheme.h
    ${CMAKE_CURRENT_LIST_DIR}/view/iconcodes.h
    ${CMAKE_CURRENT_LIST_DIR}/view/musicalsymbolcodes.h
    ${CMAKE_CURRENT_LIST_DIR}/view/qmltranslation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/qmltranslation.h
    ${CMAKE_CURRENT_LIST_DIR}/view/qmllauncher.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/qmllauncher.h
    ${CMAKE_CURRENT_LIST_DIR}/view/qmldialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/qmldialog.h
    ${CMAKE_CURRENT_LIST_DIR}/view/qmltooltip.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/qmltooltip.h

    ${CMAKE_CURRENT_LIST_DIR}/dev/interactivetestsmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dev/interactivetestsmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/dev/testdialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dev/testdialog.h
    )

set(MODULE_UI
    ${CMAKE_CURRENT_LIST_DIR}/dev/testdialog.ui
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)
