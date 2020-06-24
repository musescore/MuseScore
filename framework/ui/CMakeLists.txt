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

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/uimodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/uimodule.h
    ${CMAKE_CURRENT_LIST_DIR}/iuiengine.h
    ${CMAKE_CURRENT_LIST_DIR}/iinteractive.h
    ${CMAKE_CURRENT_LIST_DIR}/ilauncher.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/uiengine.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/uiengine.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/uiinteractive.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/uiinteractive.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/launcher.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/launcher.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/iqmllaunchprovider.h

    ${CMAKE_CURRENT_LIST_DIR}/view/qmlapi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/qmlapi.h
    ${CMAKE_CURRENT_LIST_DIR}/view/qmltheme.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/qmltheme.h
    ${CMAKE_CURRENT_LIST_DIR}/view/iconcodes.h
<<<<<<< HEAD
=======
    ${CMAKE_CURRENT_LIST_DIR}/view/qmltranslation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/qmltranslation.h
    ${CMAKE_CURRENT_LIST_DIR}/view/qmllaunchprovider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/qmllaunchprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/view/qmllauncher.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/qmllauncher.h
>>>>>>> bef82880f... mu4: added lauch provader
    )

set(MODULE_QRC ui.qrc)
set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)
