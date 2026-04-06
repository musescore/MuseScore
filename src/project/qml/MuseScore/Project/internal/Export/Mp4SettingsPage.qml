/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick
import QtQuick.Layouts

import Muse.UiComponents
import Muse.Ui
import MuseScore.Project

ExportSettingsPage {
    id: root

    property bool isFFmpegAvailable: Boolean(root.model) && root.model.isFFmpegAvailable

    isExportAvailable: root.isFFmpegAvailable

    Loader {
        width: parent.width

        sourceComponent: root.isFFmpegAvailable ? videoSettingsComp : ffmpegMissingComp
    }

    Component {
        id: ffmpegMissingComp

        Column {
            width: parent.width
            spacing: 12

            StyledTextLabel {
                width: parent.width
                text: qsTrc("project/export", "To export to this file format, an FFmpeg installation is required. You can also configure this in Preferences > Video.")
                horizontalAlignment: Text.AlignLeft
                wrapMode: Text.WordWrap
            }

            FlatButton {
                id: downloadButton

                text: qsTrc("project/export", "Download FFmpeg")
                icon: IconCode.OPEN_LINK
                isNarrow: true
                orientation: Qt.Horizontal

                navigation.name: "ExportDownloadFFmpeg"
                navigation.panel: root.navigationPanel
                navigation.row: root.navigationOrder

                onClicked: {
                    Qt.openUrlExternally("https://support.audacityteam.org/basics/installing-ffmpeg") // todo
                }
            }

            FilePicker {
                width: parent.width

                pickerType: FilePicker.PickerType.Directory
                dialogTitle: qsTrc("project/export", "Choose FFmpeg library folder")

                buttonType: FlatButton.TextOnly
                buttonText: qsTrc("project/export", "Locate existing installation")
                showPathField: false

                dir: root.model ? root.model.ffmpegDir : ""
                path: root.model ? root.model.ffmpegDir : ""

                navigation: root.navigationPanel
                navigationRowOrderStart: downloadButton.navigation.order + 1

                onPathEdited: function(newPath) {
                    if (root.model) {
                        root.model.ffmpegDir = newPath
                    }
                }
            }
        }
    }

    Component {
        id: videoSettingsComp

        Column {
            width: parent.width
            spacing: 12

            AudioSettings {
                id: audioSection

                model: root.model
                navigationPanel: root.navigationPanel
                navigationOrderStart: root.navigationOrder
                showBitRateControl: true
            }

            ExportOptionItem {
                id: videoResolutionLabel
                text: qsTrc("project/export", "Video resolution:")

                StyledDropdown {
                    Layout.preferredWidth: 126

                    navigation.name: "VideoResolutionDropdown"
                    navigation.panel: root.navigationPanel
                    navigation.row: audioSection.navigationOrderEnd + 1
                    navigation.accessible.name: videoResolutionLabel.text + " " + currentText

                    model: root.model ? root.model.availableVideoResolutions().map(function(res) {
                        return { text: res, value: res }
                    }) : []

                    currentIndex: root.model ? indexOfValue(root.model.videoResolution) : -1

                    onActivated: function(index, value) {
                        root.model.videoResolution = value
                    }
                }
            }

            StyledTextLabel {
                width: parent.width
                text: qsTrc("project/export", "Each selected part will be exported as a separate video file.")
                horizontalAlignment: Text.AlignLeft
                wrapMode: Text.WordWrap
            }
        }
    }
}
