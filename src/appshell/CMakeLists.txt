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
    ${CMAKE_CURRENT_LIST_DIR}/inotationpagestate.h
    ${CMAKE_CURRENT_LIST_DIR}/commandlinecontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/commandlinecontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/iappshellconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/appshelltypes.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/applicationuiactions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/applicationuiactions.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/appshellconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/appshellconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/applicationactioncontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/applicationactioncontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/appshellconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/appshellconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationpagestate.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/notationpagestate.h
    ${CMAKE_CURRENT_LIST_DIR}/view/devtools/settingslistmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/devtools/settingslistmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/appmenumodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/appmenumodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/notationpagemodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/notationpagemodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/aboutmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/aboutmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/startupmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/startupmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/preferences/preferencesmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/preferences/preferencesmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/preferences/preferencepageitem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/preferences/preferencepageitem.h
    ${CMAKE_CURRENT_LIST_DIR}/view/preferences/folderspreferencesmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/preferences/folderspreferencesmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/preferences/canvaspreferencesmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/preferences/canvaspreferencesmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/preferences/generalpreferencesmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/preferences/generalpreferencesmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/preferences/updatepreferencesmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/preferences/updatepreferencesmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/preferences/appearancepreferencesmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/preferences/appearancepreferencesmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/preferences/programmestartpreferencesmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/preferences/programmestartpreferencesmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/preferences/noteinputpreferencesmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/preferences/noteinputpreferencesmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/preferences/advancedpreferencesmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/preferences/advancedpreferencesmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/preferences/scorepreferencesmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/preferences/scorepreferencesmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/preferences/importpreferencesmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/preferences/importpreferencesmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/preferences/iopreferencesmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/preferences/iopreferencesmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/preferences/commonaudioapiconfigurationmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/preferences/commonaudioapiconfigurationmodel.h
    ${DOCKWINDOW_SRC}
    )

set(MODULE_LINK
    uicomponents
    ui
    )

if (KDAB_DOCKWIDGETS)
    set(MODULE_LINK
        ${MODULE_LINK}
        kddockwidgets
        )
endif(KDAB_DOCKWIDGETS)

include(${PROJECT_SOURCE_DIR}/build/module.cmake)
