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

set(MODULE appshell)

set(MODULE_QRC appshell.qrc)
set(MODULE_QML_IMPORT ${CMAKE_CURRENT_LIST_DIR}/qml)

include(${CMAKE_CURRENT_LIST_DIR}/view/dockwindow/dockwindow.cmake)

set(MODULE_SRC
    ${CMAKE_CURRENT_LIST_DIR}/appshellmodule.cpp
    ${CMAKE_CURRENT_LIST_DIR}/appshellmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/appshell.cpp
    ${CMAKE_CURRENT_LIST_DIR}/appshell.h
    ${CMAKE_CURRENT_LIST_DIR}/iapplicationactioncontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/inotationpagestate.h
    ${CMAKE_CURRENT_LIST_DIR}/commandlinecontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/commandlinecontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/iappshellconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/appshelltypes.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/applicationactions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/applicationactions.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/appshellconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/appshellconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/applicationactioncontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/applicationactioncontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/appshellconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/appshellconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationpagestate.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationpagestate.h
    ${CMAKE_CURRENT_LIST_DIR}/view/settings/settingslistmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/settings/settingslistmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/appmenumodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/appmenumodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/notationpagemodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/notationpagemodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/aboutmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/aboutmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/filemenucontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/filemenucontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/view/editmenucontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/editmenucontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/view/viewmenucontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/viewmenucontroller.h
    ${DOCKWINDOW_SRC}
    )

set(MODULE_LINK
    uicomponents
    ui
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)

