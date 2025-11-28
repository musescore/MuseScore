# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-CLA-applies
#
# MuseScore
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited and others
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

muse_create_module(muse_ui ALIAS muse::ui)

target_sources(muse_ui PRIVATE
    uimodule.cpp
    uimodule.h
    iuiengine.h
    iinteractiveprovider.h
    iinteractiveuriregister.h
    iuiconfiguration.h
    uierrors.h
    uitypes.h
    uiaction.h
    imainwindow.h
    iuiactionsregister.h
    iuiactionsmodule.h
    iuicontextresolver.h
    inavigation.h
    inavigationcontroller.h
    idragcontroller.h
    iwindowscontroller.h

    graphicsapiprovider.cpp
    graphicsapiprovider.h

    api/themeapi.cpp
    api/themeapi.h
    api/navigationapi.cpp
    api/navigationapi.h
    api/keyboardapi.cpp
    api/keyboardapi.h

    internal/interactiveuriregister.cpp
    internal/interactiveuriregister.h
    internal/uiengine.cpp
    internal/uiengine.h
    internal/mainwindow.cpp
    internal/mainwindow.h
    internal/uiconfiguration.cpp
    internal/uiconfiguration.h
    internal/themeconverter.cpp
    internal/themeconverter.h
    internal/iplatformtheme.h
    internal/uiactionsregister.cpp
    internal/uiactionsregister.h
    internal/navigationcontroller.cpp
    internal/navigationcontroller.h
    internal/navigationuiactions.cpp
    internal/navigationuiactions.h
    internal/uiarrangement.cpp
    internal/uiarrangement.h
    internal/dragcontroller.cpp
    internal/dragcontroller.h
    internal/windowscontroller.cpp
    internal/windowscontroller.h

    view/iconcodes.h
    view/interactiveprovider.cpp
    view/interactiveprovider.h
    view/mainwindowbridge.cpp
    view/mainwindowbridge.h
    view/musicalsymbolcodes.h
    view/qmlapi.cpp
    view/qmlapi.h
    view/qmldataformatter.cpp
    view/qmldataformatter.h
    view/qmllauncher.cpp
    view/qmllauncher.h
    view/qmltooltip.cpp
    view/qmltooltip.h
    view/qmltranslation.cpp
    view/qmltranslation.h
    view/widgetnavigationfix.cpp
    view/widgetnavigationfix.h
    view/widgetstatestore.cpp
    view/widgetstatestore.h
    view/widgetutils.h

    view/internal/widgetdialogadapter.cpp
    view/internal/widgetdialogadapter.h

    dev/testdialog.cpp
    dev/testdialog.h
    dev/testdialog.ui
)

if (OS_IS_MAC)
    target_sources(muse_ui PRIVATE
        internal/platform/macos/macosplatformtheme.mm
        internal/platform/macos/macosplatformtheme.h
        view/platform/macos/macosmainwindowbridge.mm
        view/platform/macos/macosmainwindowbridge.h
    )

    # Don't mix C++ and Objective-C++ in unity builds
    set_source_files_properties(
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/macos/macosplatformtheme.mm
        ${CMAKE_CURRENT_LIST_DIR}/view/platform/macos/macosmainwindowbridge.mm
        PROPERTIES
            SKIP_UNITY_BUILD_INCLUSION ON
            SKIP_PRECOMPILE_HEADERS ON
    )
elseif(OS_IS_WIN)
    target_sources(muse_ui PRIVATE
        internal/platform/windows/windowsplatformtheme.cpp
        internal/platform/windows/windowsplatformtheme.h
        internal/platform/windows/winwindowscontroller.cpp
        internal/platform/windows/winwindowscontroller.h
    )

    # Exclude from Unity Build to avoid Error C2872: 'Uri': ambiguous symbol
    set_source_files_properties(
        ${CMAKE_CURRENT_LIST_DIR}/internal/platform/windows/windowsplatformtheme.cpp
        PROPERTIES
            SKIP_UNITY_BUILD_INCLUSION ON
    )

    target_link_libraries(muse_ui PRIVATE dwmapi)
elseif(OS_IS_LIN)
    target_sources(muse_ui PRIVATE
        internal/platform/linux/linuxplatformtheme.cpp
        internal/platform/linux/linuxplatformtheme.h
    )
else()
    target_sources(muse_ui PRIVATE
        internal/platform/stub/stubplatformtheme.cpp
        internal/platform/stub/stubplatformtheme.h
    )
endif()

qt_add_resources(muse_ui "muse_ui_resources"
    PREFIX
        "/ui"
    FILES
        "data/MusescoreIcon.ttf"
)

if (QT_SUPPORT)
    target_link_libraries(muse_ui PRIVATE Qt::Quick Qt::Widgets)

    if (OS_IS_LIN)
        target_link_libraries(muse_ui PRIVATE Qt::DBus)
    endif()
endif()

if (OS_IS_MAC)
    find_library(AppKit NAMES AppKit)
    target_link_libraries(muse_ui PRIVATE ${AppKit})
endif()

if (MUSE_MODULE_UI_TESTS)
    add_subdirectory(tests)
endif()

add_subdirectory(qml/Muse/Ui)
