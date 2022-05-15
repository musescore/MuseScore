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
import QtQuick 2.7
import QtQuick.Controls 2.2

import MuseScore.UiComponents 1.0

Item {

    id: root

    property var selectedSoundFonts: []
    property var availableSoundFonts: []

    signal selectedUpClicked(var index)
    signal selectedDownClicked(var index)
    signal selectedRemoveClicked(var index)
    signal addClicked(var index)

    onSelectedSoundFontsChanged: {
        selectedView.model = 0;
        selectedView.model = root.selectedSoundFonts
    }

    Item {
        id: availablePanel

        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.margins: 8
        width: (parent.width / 2) - 40

        StyledTextLabel {
            id: headerAvailable
            anchors.left: parent.left
            anchors.leftMargin: 8
            width: parent.width
            height: 40
            font: ui.theme.bodyBoldFont
            text: "Available"
        }

        ListView {
            anchors.top: headerAvailable.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            model: root.availableSoundFonts

            delegate: Item {
                width: parent.width
                height: 40

                StyledTextLabel {
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: addBtn.left
                    anchors.leftMargin: 8

                    text: modelData
                }

                Button {
                    id: addBtn
                    height: parent.height
                    anchors.right: parent.right
                    width: 40
                    text: "→"
                    onClicked: root.addClicked(model.index)
                }
            }
        }
    }

    Rectangle {
        id: separator
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: availablePanel.right
        width: 4
        color: "#666666"
    }

    Item {
        id: selectedPanel
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.left: separator.right
        anchors.right: parent.right
        anchors.margins: 8

        StyledTextLabel {
            id: headerSelected
            anchors.left: parent.left
            anchors.leftMargin: 8
            width: parent.width
            height: 40
            font: ui.theme.bodyBoldFont
            text: "Selected"
        }

        ListView {
            id: selectedView
            anchors.top: headerSelected.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            model: root.availableSoundFonts

            delegate: Item {
                width: parent.width
                height: 40

                StyledTextLabel {
                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: btns.left
                    anchors.leftMargin: 8

                    text: modelData
                }

                Row {

                    id: btns

                    anchors.top: parent.top
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right
                    anchors.rightMargin: 8
                    width: childrenRect.width

                    Button {
                        id: upBtn
                        height: parent.height
                        width: 40
                        text: "↑"
                        onClicked: root.selectedUpClicked(model.index)
                    }

                    Button {
                        id: downBtn
                        height: parent.height
                        width: 40
                        text: "↓"
                        onClicked: root.selectedDownClicked(model.index)
                    }

                    Button {
                        id: remBtn
                        height: parent.height
                        width: 40
                        text: "-"
                        onClicked: root.selectedRemoveClicked(model.index)
                    }
                }
            }
        }
    }
}
