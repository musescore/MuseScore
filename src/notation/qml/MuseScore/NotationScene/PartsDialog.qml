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
import QtQuick 2.9
import QtQuick.Layouts 1.12

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.NotationScene 1.0

import "internal"

QmlDialog {
    id: root

    width: 664
    height: 558

    modal: true

    title: qsTrc("notation", "Parts")

    Rectangle {
        id: content

        anchors.fill: parent

        color: ui.theme.popupBackgroundColor

        PartListModel {
            id: partsModel
        }

        QtObject {
            id: privateProperties

            readonly property int sideMargin: 36
            readonly property int buttonsMargin: 24
        }

        Component.onCompleted: {
            partsModel.load()
        }

        ColumnLayout {
            anchors.fill: parent

            spacing: 0

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: childrenRect.height
                Layout.topMargin: privateProperties.sideMargin

                StyledTextLabel {
                    anchors.left: parent.left
                    anchors.leftMargin: privateProperties.sideMargin

                    text: qsTrc("notation", "Parts")
                    font: ui.theme.headerBoldFont
                }

                FlatButton {
                    text: qsTrc("notation", "Create new part")

                    anchors.right: deleteButton.left
                    anchors.rightMargin: 8

                    onClicked: {
                        partsModel.createNewPart()
                    }
                }

                FlatButton {
                    id: deleteButton

                    anchors.right: parent.right
                    anchors.rightMargin: privateProperties.buttonsMargin

                    icon: IconCode.DELETE_TANK

                    enabled: partsModel.isRemovingAvailable

                    onClicked: {
                        partsModel.removeSelectedParts()
                    }
                }
            }

            PartsView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                Layout.topMargin: 38
                Layout.bottomMargin: 24

                model: partsModel
            }

            Row {
                Layout.preferredHeight: childrenRect.height
                Layout.bottomMargin: privateProperties.buttonsMargin
                Layout.rightMargin: privateProperties.buttonsMargin
                Layout.alignment: Qt.AlignRight | Qt.AlignBottom

                spacing: 12

                FlatButton {
                    text: qsTrc("global", "Close")

                    onClicked: {
                        partsModel.apply()
                        root.hide()
                    }
                }

                FlatButton {
                    text: qsTrc("global", "Open")

                    enabled: partsModel.hasSelection

                    onClicked: {
                        partsModel.openSelectedParts()
                        partsModel.apply()
                        root.hide()
                    }
                }
            }
        }
    }
}
