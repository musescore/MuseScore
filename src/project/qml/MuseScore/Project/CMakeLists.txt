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

muse_create_qml_module(project_qml FOR project)

qt_add_qml_module(project_qml
    URI MuseScore.Project
    VERSION 1.0
    SOURCES
        audiogenerationsettingsmodel.cpp
        audiogenerationsettingsmodel.h
        enums.cpp
        enums.h
        internal/Export/exportdialogmodel.cpp
        internal/Export/exportdialogmodel.h
        internal/NewScore/additionalinfomodel.cpp
        internal/NewScore/additionalinfomodel.h
        internal/NewScore/newscoremodel.cpp
        internal/NewScore/newscoremodel.h
        internal/NewScore/templatepaintview.cpp
        internal/NewScore/templatepaintview.h
        internal/NewScore/templatesmodel.cpp
        internal/NewScore/templatesmodel.h
        internal/Properties/projectpropertiesmodel.cpp
        internal/Properties/projectpropertiesmodel.h
        internal/ScoresPage/abstractscoresmodel.cpp
        internal/ScoresPage/abstractscoresmodel.h
        internal/ScoresPage/cloudscoresmodel.cpp
        internal/ScoresPage/cloudscoresmodel.h
        internal/ScoresPage/cloudscorestatuswatcher.cpp
        internal/ScoresPage/cloudscorestatuswatcher.h
        internal/ScoresPage/pixmapscorethumbnailview.cpp
        internal/ScoresPage/pixmapscorethumbnailview.h
        internal/ScoresPage/recentscoresmodel.cpp
        internal/ScoresPage/recentscoresmodel.h
        internal/ScoresPage/scorespagemodel.cpp
        internal/ScoresPage/scorespagemodel.h
        internal/ScoresPage/scorethumbnailloader.cpp
        internal/ScoresPage/scorethumbnailloader.h
    QML_FILES
        AlsoShareAudioComDialog.qml
        AskSaveLocationTypeDialog.qml
        AudioGenerationSettings.qml
        AudioGenerationSettingsDialog.qml
        ExportDialog.qml
        internal/Export/AudioSettingsPage.qml
        internal/Export/ExportOptionItem.qml
        internal/Export/ExportOptionsView.qml
        internal/Export/ExportScoresListView.qml
        internal/Export/ExportSettingsPage.qml
        internal/Export/LrcSettingsPage.qml
        internal/Export/MeiSettingsPage.qml
        internal/Export/MidiSettingsPage.qml
        internal/Export/Mp3SettingsPage.qml
        internal/Export/MusicXmlSettingsPage.qml
        internal/Export/OggSettingsPage.qml
        internal/Export/PdfSettingsPage.qml
        internal/Export/PngSettingsPage.qml
        internal/Export/SvgSettingsPage.qml
        internal/Migration/MigrationContentFor362.qml
        internal/Migration/MigrationContentForPre362.qml
        internal/NewScore/AdditionalInfoView.qml
        internal/NewScore/ChooseInstrumentsAndTemplatesPage.qml
        internal/NewScore/CreateFromTemplatePage.qml
        internal/NewScore/GeneralInfoItem.qml
        internal/NewScore/GeneralInfoView.qml
        internal/NewScore/KeySignature.qml
        internal/NewScore/KeySignatureListView.qml
        internal/NewScore/KeySignatureSettings.qml
        internal/NewScore/MeasuresSettings.qml
        internal/NewScore/ScoreInfoPage.qml
        internal/NewScore/TemplatePreview.qml
        internal/NewScore/TempoSettings.qml
        internal/NewScore/TimeSignatureFraction.qml
        internal/NewScore/TimeSignatureSettings.qml
        internal/NewScore/TimeSignatureView.qml
        internal/NewScore/TitleListView.qml
        internal/Properties/ProjectPropertiesFileInfoPanel.qml
        internal/Properties/ProjectPropertiesView.qml
        internal/Properties/PropertyItem.qml
        internal/SaveToCloud/SaveLocationOption.qml
        internal/ScoresPage/CloudScoreIndicatorButton.qml
        internal/ScoresPage/RecentScoresView.qml
        internal/ScoresPage/ScoreGridItem.qml
        internal/ScoresPage/ScoreListItem.qml
        internal/ScoresPage/ScoreThumbnail.qml
        Message.qml
        MigrationDialog.qml
        NewScoreDialog.qml
        ProjectPropertiesDialog.qml
        ProjectUploadedDialog.qml
        SaveToCloudDialog.qml
        ScoresGridView.qml
        ScoresListView.qml
        ScoresPage.qml
        ScoresView.qml
        UploadProgressDialog.qml
    RESOURCES
        internal/Migration/migration.png
        internal/SaveToCloud/images/Cloud.png
        internal/SaveToCloud/images/Laptop.png
        internal/ScoresPage/images/Placeholder_GP.png
        internal/ScoresPage/images/Placeholder_MEI.png
        internal/ScoresPage/images/Placeholder_MIDI.png
        internal/ScoresPage/images/Placeholder_MXML.png
        internal/ScoresPage/images/Placeholder_Other.png
        resources/AudioCom_Waveform.png
        resources/PublishScores.png
    IMPORTS
        TARGET muse_ui_qml
        TARGET muse_uicomponents_qml
        TARGET notationscene_qml
)

fixup_qml_module_dependencies(project_qml)

# Necessary for the auto-generated sources
target_include_directories(project_qml PRIVATE
    internal
    internal/Export
    internal/NewScore
    internal/Properties
    internal/ScoresPage
)
