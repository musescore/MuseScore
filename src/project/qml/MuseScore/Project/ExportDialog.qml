/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Project 1.0

import "internal/Export"

StyledDialogView {
    id: root

    title: qsTrc("project/export", "Export")

    contentWidth: 756
    contentHeight: 372
    margins: 24

    ExportDialogModel {
        id: exportModel
    }

    Component.onCompleted: {
        exportModel.load()
    }

    onNavigationActivateRequested: {
        exportScoresListView.focusOnFirst()
    }

    onClosed: {
        exportModel.updateExportInfo()
    }

    RowLayout {
        anchors.fill: parent
        spacing: 2 * root.margins

        ColumnLayout {
            Layout.fillWidth: true
            Layout.preferredWidth: (parent.width - parent.spacing) / 2
            spacing: 18

            StyledTextLabel {
                text: qsTrc("project/export", "Select parts to export")
                font: ui.theme.bodyBoldFont
            }

            ExportScoresListView {
                id: exportScoresListView
                Layout.fillWidth: true
                Layout.fillHeight: true

                scoresModel: exportModel
                navigation.section: root.navigationSection
                navigation.order: 1
            }

            RowLayout {
                spacing: 12

                NavigationPanel {
                    id: leftButtonsNavPanel
                    name: "Export dialog left buttons"
                    section: root.navigationSection
                    order: 2
                    direction: NavigationPanel.Horizontal
                }

                FlatButton {
                    Layout.fillWidth: true

                    text: qsTrc("project/export", "Select all")

                    navigation.name: "Select all"
                    navigation.panel: leftButtonsNavPanel
                    navigation.column: 1

                    onClicked: {
                        exportModel.setAllSelected(true)
                    }
                }

                FlatButton {
                    Layout.fillWidth: true

                    text: qsTrc("project/export", "Clear selection")

                    navigation.name: "Clear selection"
                    navigation.panel: leftButtonsNavPanel
                    navigation.order: 2

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
                text: qsTrc("project/export", "Export settings")
                font: ui.theme.bodyBoldFont
            }

            ExportOptionsView {
                Layout.fillHeight: true
                Layout.fillWidth: true

                exportModel: exportModel
                navigation.section: root.navigationSection
                navigation.order: 3
            }

            RowLayout {
                Layout.alignment: Qt.AlignRight
                spacing: 12

                ButtonBox {
                    Layout.fillWidth: true

                    buttons: [ ButtonBoxModel.Cancel ]

                    navigationPanel.section: root.navigationSection
                    navigationPanel.order: 4

                    FlatButton {
                        text: qsTrc("project/export", "Export…")
                        buttonRole: ButtonBoxModel.AcceptRole
                        buttonId: ButtonBoxModel.Done
                        enabled: exportModel.selectionLength > 0
                        accentButton: true

                        onClicked: {
                            if (exportModel.exportScores()) {
                                root.hide();
                            }
                        }
                    }

                    onStandardButtonClicked: function(buttonId) {
                        if (buttonId === ButtonBoxModel.Cancel) {
                            root.hide()
                        }
                    }
                }
            }
        }
    }
}
