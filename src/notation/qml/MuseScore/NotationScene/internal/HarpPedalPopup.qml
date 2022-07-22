/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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
import MuseScore.NotationScene 1.0

StyledPopupView {
    id: root

    HarpPedalPopupModel {
        id: harpModel

        onIsDiagramChanged: updatePosition(harpModel.pos, harpModel.size)
    }

    property QtObject model: harpModel

    property variant pedalState: harpModel.pedalState

    contentWidth: menuItems.width

    contentHeight: menuItems.height

    margins: 0

    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    showArrow: false

    function updatePosition(pos, size) {

        // Default: open above position of diagram
        setOpensUpward(true)
        root.x = (pos.x + size.x / 2) - contentWidth / 2
        root.y = pos.y - size.y - contentHeight


        // For diagrams below stave, position above stave to not obscure it
        if (harpModel.belowStave) {
            root.y = harpModel.staffPos.y - size.y - contentHeight
        }

        // not enough room on window to open above so open below stave
        var globPos = mapToItem(notationView.windowContent, Qt.point(root.x, root.y))
        if (globPos.y < 0) {
            setOpensUpward(false)
            root.y = harpModel.staffPos.y + harpModel.staffHeight
        }

        // not enough room below stave to open so open above
        if (root.y > notationView.height) {
            root.y = pos.y - size.y
        }
    }

    ColumnLayout {
        id: menuItems

        RowLayout {
            id: pedalSettings
            Layout.preferredHeight: 120
            Layout.topMargin: 15

            NavigationPanel {
                id: pedalSettingsNavPanel
                name: "PedalSettings"
                direction: NavigationPanel.Vertical
                section: root.navigationSection
                order: 1
                accessible.name: qsTrc("notation", "Pedal Settings buttons")
            }

            // Dummy column
            ColumnLayout {
                Layout.preferredWidth: 16
                Layout.preferredHeight: 120
                Item {

                }
            }

            ColumnLayout {
                id: accidentalLabels
                Layout.preferredWidth: 20
                Layout.preferredHeight: 120

                Item {
                    Layout.preferredHeight: 20

                }

                StyledIconLabel {
                    Layout.preferredHeight: 20
                    iconCode: IconCode.FLAT
                    Layout.alignment: Qt.AlignCenter
                }

                StyledIconLabel {
                    Layout.preferredHeight: 20
                    iconCode: IconCode.NATURAL
                    Layout.alignment: Qt.AlignCenter
                }

                StyledIconLabel {
                    Layout.preferredHeight: 20
                    iconCode: IconCode.SHARP
                    Layout.alignment: Qt.AlignCenter
                }
            }

            Repeater {
                width: parent.width

                model: [
                    { buttonsId: "DStringButtons", stringId: 0, stringName: "D" },
                    { buttonsId: "CStringButtons", stringId: 1, stringName: "C" },
                    { buttonsId: "BStringButtons", stringId: 2, stringName: "B" },
                ]

                HarpPedalGridColumn {
                    repeaterModel: modelData
                    model: harpModel
                    navPanel: pedalSettingsNavPanel
                }
            }

            ColumnLayout {
                Layout.preferredWidth: 20
                Layout.preferredHeight: 120
                SeparatorLine {
                    orientation: Qt.Vertical
                    Layout.alignment: Qt.AlignCenter
                }
            }

            Repeater {
                width: parent.width

                model: [
                    { buttonsId: "EStringButtons", stringId: 3, stringName: "E" },
                    { buttonsId: "FStringButtons", stringId: 4, stringName: "F" },
                    { buttonsId: "GStringButtons", stringId: 5, stringName: "G" },
                    { buttonsId: "AStringButtons", stringId: 6, stringName: "A" },
                ]

                HarpPedalGridColumn {
                    repeaterModel: modelData
                    model: harpModel
                    navPanel: pedalSettingsNavPanel
                }
            }

            // Dummy column
            ColumnLayout {
                Layout.preferredWidth: 16
                Layout.preferredHeight: 120
                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }
            }
        }

        // Diagram or text options
        RowLayout {
            id: isDiagramButtons
            width: parent.width
            Layout.topMargin: 10
            Layout.bottomMargin: 15

            NavigationPanel {
                id: navDiagramPanel
                name: "HarpPedalIsDiagramButtons"
                section: root.navigationSection
                direction: NavigationPanel.Horizontal
                order: 2
                accessible.name: qsTrc("notation", "isDiagram buttons")
            }

            Item {
                Layout.preferredWidth: 20
            }

            RoundedRadioButton {
                id: diagramButton
                Layout.fillWidth: true

                checked: model.isDiagram
                text: qsTrc("notation", "Diagram")

                navigation.name: "diagramButton"
                navigation.panel: navDiagramPanel
                navigation.order: 1
                navigation.accessible.name: qsTrc("notation", "Diagram")

                onToggled: {
                    model.setIsDiagram(true)
                }
            }

            RoundedRadioButton {
                id: textButton
                Layout.fillWidth: true

                checked: !model.isDiagram
                text: qsTrc("notation", "Text")

                navigation.name: "textButton"
                navigation.panel: navDiagramPanel
                navigation.order: 2
                navigation.accessible.name: qsTrc("notation", "Text")

                onToggled: {
                    model.setIsDiagram(false)
                }
            }
        }
    }
}
