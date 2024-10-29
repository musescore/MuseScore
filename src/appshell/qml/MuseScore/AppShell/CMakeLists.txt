# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2024 MuseScore Limited
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

# Workaround for https://bugreports.qt.io/browse/QTBUG-122702
# preferred way would be no extra `appshellqml` library, and just passing `appshell` to `qt_add_qml_module`
qt_add_library(appshellqml)

qt_add_qml_module(appshellqml
    URI MuseScore.AppShell
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
        DevTools/Interactive/SampleDialog.qml
        DevTools/KeyNav/KeyNavExample.qml
        DevTools/KeyNav/KeyNavSection.qml
        DevTools/KeyNav/KeyNavSubSection.qml
        DevTools/MPE/ArticulationsProfileEditorView.qml
        DevTools/Preferences/SettingsPage.qml
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
        Preferences/AdvancedPreferencesPage.qml
        Preferences/AppearancePreferencesPage.qml
        Preferences/AudioMidiPreferencesPage.qml
        Preferences/BraillePreferencesPage.qml
        Preferences/CanvasPreferencesPage.qml
        Preferences/FoldersPreferencesPage.qml
        Preferences/GeneralPreferencesPage.qml
        Preferences/ImportPreferencesPage.qml
        Preferences/MidiDeviceMappingPreferencesPage.qml
        Preferences/NoteInputPreferencesPage.qml
        Preferences/PreferencesDialog.qml
        Preferences/PreferencesPage.qml
        Preferences/SaveAndPublishPreferencesPage.qml
        Preferences/ScorePreferencesPage.qml
        Preferences/ShortcutsPreferencesPage.qml
        Preferences/UpdatePreferencesPage.qml
        Preferences/internal/AccentColorsSection.qml
        Preferences/internal/AdvancedTopSection.qml
        Preferences/internal/AudioApiSection.qml
        Preferences/internal/AudioEngineSection.qml
        Preferences/internal/AutoSaveSection.qml
        Preferences/internal/AutomaticUpdateSection.qml
        Preferences/internal/BaseSection.qml
        Preferences/internal/BrailleAdvancedSection.qml
        Preferences/internal/BrailleSection.qml
        Preferences/internal/CharsetsSection.qml
        Preferences/internal/ColorAndWallpaperSection.qml
        Preferences/internal/ComboBoxWithTitle.qml
        Preferences/internal/CommonAudioApiConfiguration.qml
        Preferences/internal/DefaultFilesSection.qml
        Preferences/internal/FoldersSection.qml
        Preferences/internal/ImportStyleSection.qml
        Preferences/internal/IncrementalPropertyControlWithTitle.qml
        Preferences/internal/KeyboardLayoutsSection.qml
        Preferences/internal/LanguagesSection.qml
        Preferences/internal/MeiSection.qml
        Preferences/internal/MidiDevicesSection.qml
        Preferences/internal/MidiSection.qml
        Preferences/internal/MiscellaneousSection.qml
        Preferences/internal/MixerSection.qml
        Preferences/internal/MusicXmlSection.qml
        Preferences/internal/NoteInputPlaySection.qml
        Preferences/internal/NoteInputSection.qml
        Preferences/internal/PreferencesButtonsPanel.qml
        Preferences/internal/PreferencesMenu.qml
        Preferences/internal/ProgramStartSection.qml
        Preferences/internal/PublishMuseScoreComSection.qml
        Preferences/internal/RemoteControlSection.qml
        Preferences/internal/SaveToCloudSection.qml
        Preferences/internal/ScrollPagesSection.qml
        Preferences/internal/ThemeAdditionalOptionsSection.qml
        Preferences/internal/ThemesSection.qml
        Preferences/internal/UiColorsSection.qml
        Preferences/internal/UiFontSection.qml
        Preferences/internal/ZoomSection.qml
        PublishPage/PublishPage.qml
        PublishPage/PublishToolBar.qml
        WindowContent.qml
        platform/AppButtonBackground.qml
        platform/AppMenuBar.qml
        platform/PlatformMenuBar.qml
        platform/linux/Main.qml
        platform/mac/Main.qml
        platform/wasm/Main.qml
        platform/win/AppSystemButtons.qml
        platform/win/AppTitleBar.qml
        platform/win/Main.qml
        shared/AccentColorsList.qml
        shared/ThemeSamplesList.qml
        shared/internal/ThemeSample.qml
    RESOURCES
        FirstLaunchSetup/resources/MuseSounds.png
        FirstLaunchSetup/resources/VideoTutorials.png
        resources/mu_logo.svg
)
