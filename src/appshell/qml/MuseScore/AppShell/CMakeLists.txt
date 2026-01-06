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

muse_create_qml_module(appshell_qml FOR appshell)

qt_add_qml_module(appshell_qml
    URI MuseScore.AppShell
    VERSION 1.0
    SOURCES
        aboutmodel.cpp
        aboutmodel.h
        appmenumodel.cpp
        appmenumodel.h
        DevTools/Settings/settingslistmodel.cpp
        DevTools/Settings/settingslistmodel.h
        FirstLaunchSetup/firstlaunchsetupmodel.cpp
        FirstLaunchSetup/firstlaunchsetupmodel.h
        FirstLaunchSetup/themespagemodel.cpp
        FirstLaunchSetup/themespagemodel.h
        FirstLaunchSetup/tutorialspagemodel.cpp
        FirstLaunchSetup/tutorialspagemodel.h
        maintoolbarmodel.cpp
        maintoolbarmodel.h
        mainwindowtitleprovider.cpp
        mainwindowtitleprovider.h
        notationpagemodel.cpp
        notationpagemodel.h
        notationstatusbarmodel.cpp
        notationstatusbarmodel.h
        PublishPage/publishtoolbarmodel.cpp
        PublishPage/publishtoolbarmodel.h
        welcomedialogmodel.cpp
        welcomedialogmodel.h
    QML_FILES
        AboutDialog.qml
        AboutMusicXMLDialog.qml
        AppWindow.qml
        DevTools/CorruptScore/CorruptScoreDevTools.qml
        DevTools/CrashHandler/CrashHandlerDevTools.qml
        DevTools/DevToolsMenu.qml
        DevTools/DevToolsPage.qml
        DevTools/Extensions/ExtensionsListView.qml
        DevTools/Gallery/GeneralComponentsGallery.qml
        DevTools/Interactive/InteractiveTests.qml
        DevTools/KeyNav/KeyNavExample.qml
        DevTools/KeyNav/KeyNavSection.qml
        DevTools/KeyNav/KeyNavSubSection.qml
        DevTools/MPE/ArticulationsProfileEditorView.qml
        DevTools/Settings/SettingsPage.qml
        FirstLaunchSetup/FirstLaunchSetupDialog.qml
        FirstLaunchSetup/Page.qml
        FirstLaunchSetup/PlaybackPage.qml
        FirstLaunchSetup/ThemesPage.qml
        FirstLaunchSetup/TutorialsPage.qml
        HomePage/HomeMenu.qml
        HomePage/HomePage.qml
        HomePage/PluginsPage.qml
        MainToolBar.qml
        NotationPage/NotationPage.qml
        NotationPage/NotationStatusBar.qml
        platform/AppButtonBackground.qml
        platform/AppMenuBar.qml
        platform/PlatformMenuBar.qml
        PublishPage/PublishPage.qml
        PublishPage/PublishToolBar.qml
        WelcomeDialog.qml
        WindowContent.qml
    RESOURCES
        FirstLaunchSetup/resources/MuseSounds.png
        FirstLaunchSetup/resources/VideoTutorials.png
        resources/mu_logo.svg
    IMPORTS
        TARGET muse_ui_qml
        TARGET muse_uicomponents_qml
        TARGET muse_cloud_qml
        TARGET muse_dockwindow_qml
        TARGET muse_diagnostics_qml
        TARGET muse_extensions_qml
        TARGET muse_learn_qml
        TARGET muse_mpe_qml
        TARGET muse_shortcuts_qml
        TARGET muse_tours_qml
        TARGET engraving_qml
        TARGET inspector_qml
        TARGET instrumentsscene_qml
        TARGET musesounds_qml
        TARGET notationscene_qml
        TARGET palette_qml
        TARGET playback_qml
        TARGET preferences_qml
        TARGET project_qml
)

if (OS_IS_MAC)
    qt_target_qml_sources(appshell_qml 
        QML_FILES
            platform/mac/Main.qml
    )
elseif(OS_IS_WIN)
    qt_target_qml_sources(appshell_qml 
        QML_FILES
            platform/win/AppSystemButtons.qml
            platform/win/AppTitleBar.qml
            platform/win/Main.qml
    )
elseif(OS_IS_LIN OR OS_IS_FREEBSD)
    qt_target_qml_sources(appshell_qml 
        QML_FILES
            platform/linux/Main.qml
    )
else()
    message(FATAL_ERROR "Unsupported OS")
endif()

if (NOT OS_IS_MAC)
    target_sources(appshell_qml PRIVATE
        navigableappmenumodel.cpp
        navigableappmenumodel.h
    )

    # we use QKeyMapper to fix https://github.com/musescore/MuseScore/issues/10181
    find_package(Qt6 REQUIRED COMPONENTS GuiPrivate)
    target_link_libraries(appshell_qml PRIVATE Qt6::GuiPrivate)
endif(NOT OS_IS_MAC)

fixup_qml_module_dependencies(appshell_qml)

# Not adding this to the QML module because it isn't really part of the QML code
qt_add_resources(appshell_qml appshell_qml_resources
    PREFIX
        /
    FILES
        resources/OnlineSoundsPreview.gif
)

# Necessary for the auto-generated sources
target_include_directories(appshell_qml PRIVATE
    DevTools/Settings
    FirstLaunchSetup
    PublishPage
)
