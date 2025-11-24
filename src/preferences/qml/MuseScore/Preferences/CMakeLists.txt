# SPDX-License-Identifier: GPL-3.0-only
# MuseScore-Studio-CLA-applies
#
# MuseScore Studio
# Music Composition & Notation
#
# Copyright (C) 2021 MuseScore Limited
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

muse_create_qml_module(preferences_qml FOR preferences)

target_link_libraries(preferences_qml PRIVATE Qt::Core5Compat)

qt_add_qml_module(preferences_qml
    URI MuseScore.Preferences
    VERSION 1.0
    SOURCES
        advancedpreferencesmodel.cpp
        advancedpreferencesmodel.h
        appearancepreferencesmodel.cpp
        appearancepreferencesmodel.h
        audiomidipreferencesmodel.cpp
        audiomidipreferencesmodel.h
        braillepreferencesmodel.cpp
        braillepreferencesmodel.h
        canvaspreferencesmodel.cpp
        canvaspreferencesmodel.h
        commonaudioapiconfigurationmodel.cpp
        commonaudioapiconfigurationmodel.h
        folderspreferencesmodel.cpp
        folderspreferencesmodel.h
        generalpreferencesmodel.cpp
        generalpreferencesmodel.h
        importpreferencesmodel.cpp
        importpreferencesmodel.h
        noteinputpreferencesmodel.cpp
        noteinputpreferencesmodel.h
        percussionpreferencesmodel.cpp
        percussionpreferencesmodel.h
        preferencepageitem.cpp
        preferencepageitem.h
        preferencesmodel.cpp
        preferencesmodel.h
        saveandpublishpreferencesmodel.cpp
        saveandpublishpreferencesmodel.h
        scorepreferencesmodel.cpp
        scorepreferencesmodel.h
        updatepreferencesmodel.cpp
        updatepreferencesmodel.h
    QML_FILES
        AccentColorsList.qml
        AdvancedPreferencesPage.qml
        AppearancePreferencesPage.qml
        AudioMidiPreferencesPage.qml
        BraillePreferencesPage.qml
        CanvasPreferencesPage.qml
        FoldersPreferencesPage.qml
        GeneralPreferencesPage.qml
        ImportPreferencesPage.qml
        MidiDeviceMappingPreferencesPage.qml
        NoteInputPreferencesPage.qml
        PercussionPreferencesPage.qml
        PreferencesDialog.qml
        PreferencesPage.qml
        SaveAndPublishPreferencesPage.qml
        ScorePreferencesPage.qml
        ShortcutsPreferencesPage.qml
        ThemeSamplesList.qml
        UpdatePreferencesPage.qml

        internal/AccentColorsSection.qml
        internal/AdvancedTopSection.qml
        internal/AudioApiSection.qml
        internal/AudioEngineSection.qml
        internal/AutomaticUpdateSection.qml
        internal/AutoSaveSection.qml
        internal/BaseSection.qml
        internal/BrailleAdvancedSection.qml
        internal/BrailleSection.qml
        internal/CharsetsSection.qml
        internal/ColorAndWallpaperSection.qml
        internal/ComboBoxWithTitle.qml
        internal/CommonAudioApiConfiguration.qml
        internal/DefaultStyleSection.qml
        internal/FoldersSection.qml
        internal/ImportStyleSection.qml
        internal/IncrementalPropertyControlWithTitle.qml
        internal/KeyboardLayoutsSection.qml
        internal/LanguagesSection.qml
        internal/MeiSection.qml
        internal/MidiDevicesSection.qml
        internal/MidiSection.qml
        internal/MiscellaneousSection.qml
        internal/MixerSection.qml
        internal/MusicXmlSection.qml
        internal/NoteInput/FretboardDiagramsSection.qml
        internal/NoteInput/MidiInputSection.qml
        internal/NoteInput/NoteColorsSection.qml
        internal/NoteInput/NoteInputSection.qml
        internal/NoteInput/NotePreviewSection.qml
        internal/NoteInput/VoiceAssignmentSection.qml
        internal/OnlineSoundsSection.qml
        internal/PageSection.qml
        internal/PreferencesButtonsPanel.qml
        internal/PreferencesMenu.qml
        internal/ProgramStartSection.qml
        internal/PublishMuseScoreComSection.qml
        internal/RemoteControlSection.qml
        internal/SaveToCloudSection.qml
        internal/ScrollPagesSection.qml
        internal/ThemeAdditionalOptionsSection.qml
        internal/ThemesSection.qml
        internal/ThemeSample.qml
        internal/UiColorsSection.qml
        internal/UiFontSection.qml
        internal/WelcomeDialogSection.qml
        internal/ZoomSection.qml
    IMPORTS
        TARGET muse_uicomponents_qml
)
