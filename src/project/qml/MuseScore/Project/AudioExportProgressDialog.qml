/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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

import Muse.Ui
import Muse.UiComponents
import Muse.Interactive
import MuseScore.Project

StyledDialogView {
    id: root

    contentWidth: 420
    contentHeight: content.implicitHeight

    margins: 16

    modal: true
    frameless: true
    closeOnEscape: false

    AudioExportProgressModel {
        id: exportModel
    }

    ProgressDialogModel {
        id: overallModel

        onFinished: {
            root.close()
        }
    }

    Component.onCompleted: {
        exportModel.load()
        overallModel.load(exportModel.overallProgress)
    }

    ColumnLayout {
        id: content

        width: root.contentWidth
        spacing: 12

        StyledTextLabel {
            Layout.fillWidth: true

            text: qsTrc("project/export", "Exporting audio…")
            font: ui.theme.largeBodyBoldFont
            horizontalAlignment: Text.AlignLeft
        }

        StyledTextLabel {
            Layout.fillWidth: true

            visible: !isEmpty
            text: overallModel.statusMessage
            horizontalAlignment: Text.AlignLeft
        }

        ProgressBar {
            Layout.fillWidth: true
            Layout.preferredHeight: 30

            from: 0
            value: overallModel.value
            to: overallModel.to

            progressStatus: overallModel.to != 0 ? Math.round(overallModel.value * 100 / overallModel.to) + "%" : "0%"
        }

        StyledFlickable {
            id: filesView

            Layout.fillWidth: true
            Layout.preferredHeight: Math.min(filesColumn.implicitHeight, 280)

            contentWidth: width
            contentHeight: filesColumn.implicitHeight

            Column {
                id: filesColumn

                width: filesView.width
                spacing: 6

                //! NOTE A Repeater (not a list view) so the rows, and the progress they follow, stay alive while scrolled away
                Repeater {
                    model: exportModel.files

                    RowLayout {
                        width: filesColumn.width
                        spacing: 12

                        ProgressDialogModel {
                            id: fileModel
                        }

                        Component.onCompleted: {
                            fileModel.load(modelData.progress)
                        }

                        StyledTextLabel {
                            Layout.preferredWidth: 140

                            text: modelData.name
                            horizontalAlignment: Text.AlignLeft
                        }

                        ProgressBar {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 16

                            from: 0
                            value: fileModel.value
                            to: fileModel.to > 0 ? fileModel.to : 100

                            progressStatus: ""
                        }
                    }
                }
            }
        }

        FlatButton {
            Layout.alignment: Qt.AlignRight

            text: qsTrc("global", "Cancel")

            onClicked: {
                overallModel.cancel()
                root.reject()
            }
        }
    }
}
