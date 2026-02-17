# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2026 MuseScore Limited
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
# along with this program. If not, see <https://www.gnu.org/licenses/>.

muse_create_module(appshell)

target_sources(appshell PRIVATE
    appshellmodule.cpp
    appshellmodule.h
    iappshellconfiguration.h
    appshelltypes.h
    internal/applicationactioncontroller.cpp
    internal/applicationactioncontroller.h
    internal/applicationuiactions.cpp
    internal/applicationuiactions.h
    internal/appshellconfiguration.cpp
    internal/appshellconfiguration.h
    internal/iappmenumodelhook.h
    internal/isessionsmanager.h
    internal/istartupscenario.h
    internal/sessionsmanager.cpp
    internal/sessionsmanager.h
    internal/startupscenario.cpp
    internal/startupscenario.h
)

if (OS_IS_MAC)
    target_sources(appshell PRIVATE
        internal/platform/macos/macosappmenumodelhook.mm
        internal/platform/macos/macosappmenumodelhook.h
    )

    set_source_files_properties(
        internal/platform/macos/macosappmenumodelhook.mm
        PROPERTIES
        SKIP_UNITY_BUILD_INCLUSION ON
        SKIP_PRECOMPILE_HEADERS ON
    )
endif(OS_IS_MAC)

qt_add_resources(appshell appshell_resources
    PREFIX
        /
    FILES
        resources/welcomedialog/ExploreTutorials.png
        resources/welcomedialog/MuseScoreCom.png
        resources/welcomedialog/MuseSounds.png
        resources/welcomedialog/WhatsNew.png
        resources/win_opengl_buglist.json
)

target_link_libraries(appshell PRIVATE
    muse::ui
    muse::uicomponents
    Qt::Quick
    Qt::Svg
    Qt::Widgets
)

if (MUSE_MODULE_DOCKWINDOW)
    target_link_libraries(appshell PRIVATE muse::dockwindow)
endif()

if (MUE_BUILD_APPSHELL_QML)
    add_subdirectory(qml/MuseScore/AppShell)
endif()
