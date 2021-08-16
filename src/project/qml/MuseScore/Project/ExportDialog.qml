/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Project 1.0

import "internal/Export"

StyledDialogView {
    id: root

    title: qsTrc("project", "Export")

    contentWidth: 756
    contentHeight: 386
    margins: privateProperties.sideMargin

    modal: true

    ExportDialogModel {
        id: exportModel
    }

    QtObject {
        id: privateProperties

        readonly property int sideMargin: 24
    }

    Component.onCompleted: {
        exportModel.load();
    }

    RowLayout {
        anchors.fill: parent
        spacing: 2 * privateProperties.sideMargin

        ColumnLayout {
            Layout.fillWidth: true
            Layout.preferredWidth: parent.width / 2 - parent.spacing / 2
            spacing: 18

            StyledTextLabel {
                text: qsTrc("project", "Select parts to export")
                font: ui.theme.bodyBoldFont
            }

            ExportScoresListView {
                id: exportScoresListView
                Layout.fillWidth: true
                Layout.fillHeight: true

                scoresModel: exportModel
            }

            RowLayout {
                Layout.topMargin: 10
                spacing: 12

                FlatButton {
                    Layout.fillWidth: true

                    text: qsTrc("project", "Select all")

                    onClicked: {
                        exportModel.setAllSelected(true)
                    }
                }

                FlatButton {
                    Layout.fillWidth: true

                    text: qsTrc("project", "Clear selection")

                    onClicked: {
                        exportModel.setAllSelected(false)
                    }
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.preferredWidth: parent.width / 2 - parent.spacing / 2
            spacing: 18

            StyledTextLabel {
                text: qsTrc("project", "Export settings")
                font: ui.theme.bodyBoldFont
            }

            Column {
                Layout.fillHeight: true
                Layout.fillWidth: true

                ExportOptionsView {
                    id: exportOptionsView
                    width: parent.width

                    exportModel: exportModel
                }
            }

            RowLayout {
                Layout.alignment: Qt.AlignRight
                Layout.topMargin: 10
                spacing: 12

                FlatButton {
                    text: qsTrc("global", "Cancel")
                    accentButton: !exportButton.enabled

                    onClicked: {
                        root.hide()
                    }
                }

                FlatButton {
                    id: exportButton

                    text: qsTrc("project", "Export…")
                    enabled: exportModel.selectionLength > 0;
                    accentButton: enabled
                    onClicked: {
                        if (exportModel.exportScores()) {
                            root.hide();
                        }
                    }
                }
            }
        }
    }
}
