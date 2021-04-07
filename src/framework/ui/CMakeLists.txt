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

include(GetPlatformInfo)
if (OS_IS_MAC)
    set(PLATFORM_THEME_SRC
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/macos/macosplatformtheme.mm
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/macos/macosplatformtheme.h
    )
    # Don't mix C++ and Objective-C++ in Unity Build
    set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/internal/platform/macos/macosplatformtheme.mm
                                PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)

    set_source_files_properties(${CMAKE_CURRENT_LIST_DIR}/internal/platform/macos/macosplatformtheme.mm
                                PROPERTIES SKIP_PRECOMPILE_HEADERS ON)
elseif(OS_IS_WIN)
    set(PLATFORM_THEME_SRC
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/windows/windowsplatformtheme.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/windows/windowsplatformtheme.h
    )
else()
    set(PLATFORM_THEME_SRC
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/stub/stubplatformtheme.cpp
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/stub/stubplatformtheme.h
    )
endif()

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
    ${CMAKE_CURRENT_LIST_DIR}/iuiactionsregister.h
    ${CMAKE_CURRENT_LIST_DIR}/iuiactionsmodule.h
    ${CMAKE_CURRENT_LIST_DIR}/iuicontextresolver.h
    ${CMAKE_CURRENT_LIST_DIR}/ikeynavigation.h
    ${CMAKE_CURRENT_LIST_DIR}/ikeynavigationcontroller.h

    ${CMAKE_CURRENT_LIST_DIR}/internal/interactiveuriregister.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/interactiveuriregister.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/uiengine.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/uiengine.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/uiconfiguration.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/uiconfiguration.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/themeconverter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/themeconverter.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/iplatformtheme.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/uiactionsregister.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/uiactionsregister.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/keynavigationcontroller.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/keynavigationcontroller.h
    ${CMAKE_CURRENT_LIST_DIR}/internal/keynavigationuiactions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/internal/keynavigationuiactions.h
    ${PLATFORM_THEME_SRC}

    ${CMAKE_CURRENT_LIST_DIR}/view/interactiveprovider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/interactiveprovider.h
    ${CMAKE_CURRENT_LIST_DIR}/view/qmlapi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/qmlapi.h
    ${CMAKE_CURRENT_LIST_DIR}/view/uitheme.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/uitheme.h
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
    ${CMAKE_CURRENT_LIST_DIR}/view/widgetdialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/widgetdialog.h
    ${CMAKE_CURRENT_LIST_DIR}/view/abstractmenumodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/abstractmenumodel.h
    ${CMAKE_CURRENT_LIST_DIR}/view/abstractkeynavigation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/abstractkeynavigation.h
    ${CMAKE_CURRENT_LIST_DIR}/view/keynavigationsection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/keynavigationsection.h
    ${CMAKE_CURRENT_LIST_DIR}/view/keynavigationsubsection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/keynavigationsubsection.h
    ${CMAKE_CURRENT_LIST_DIR}/view/keynavigationcontrol.cpp
    ${CMAKE_CURRENT_LIST_DIR}/view/keynavigationcontrol.h

    ${CMAKE_CURRENT_LIST_DIR}/dev/interactivetestsmodel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dev/interactivetestsmodel.h
    ${CMAKE_CURRENT_LIST_DIR}/dev/testdialog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/dev/testdialog.h
    )

set(MODULE_UI
    ${CMAKE_CURRENT_LIST_DIR}/dev/testdialog.ui
    )

include(${PROJECT_SOURCE_DIR}/build/module.cmake)
