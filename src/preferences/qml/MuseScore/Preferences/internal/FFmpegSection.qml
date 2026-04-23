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

BaseSection {
    id: root

    title: qsTrc("preferences", "Video converter")

    property var model: null

    ColumnLayout {
        width: parent.width
        height: 30

        spacing: 12

        StyledTextLabel {
            id: titleLabel
            Layout.fillWidth: true
            text: qsTrc("preferences", "FFmpeg library version:") + " " + (model && model.ffmpegVersion !== -1
                                                                          ? model.ffmpegVersion
                                                                          : qsTrc("preferences", "FFmpeg library not found"))
            horizontalAlignment: Text.AlignLeft
        }

        RowLayout {
            Layout.fillWidth: true
            height: 24

            spacing: 24

            FilePicker {
                id: ffmpegDirPicker
                Layout.fillWidth: true

                pickerType: FilePicker.PickerType.Directory
                dialogTitle: qsTrc("preferences", "Choose %1 folder").arg(root.title)

                buttonType: FlatButton.TextOnly
                buttonText: qsTrc("preferences", "Locate existing installation")

                dir: model ? model.ffmpegDir : ""
                path: model ? model.ffmpegDir : ""

                navigation: root.navigation
                pathFieldTitle: titleLabel.text

                spacing: 8

                onPathEdited: function(newPath) {
                    if (model) {
                        model.ffmpegDir = newPath
                    }
                }
            }

            FlatButton {
                text: qsTrc("preferences", "Download FFmpeg")
                icon: IconCode.OPEN_LINK
                isNarrow: true
                orientation: Qt.Horizontal

                navigation.panel: root.navigation
                navigation.row: ffmpegDirPicker.navigationRowOrderStart
                navigation.column: 100

                onClicked: {
                    api.launcher.openUrl("https://handbook.musescore.org/video/installing-ffmpeg")
                }
            }
        }
    }
}
