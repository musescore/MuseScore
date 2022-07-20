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
    }

    property QtObject model: harpModel

    property NavigationSection navigationSection: null

    property NavigationPanel navigationPanel: NavigationPanel {
        name: "HarpDiagramPopup"
        section: root.navigationSection
        order: 1
        direction: NavigationPanel.Vertical
    }

    width: 330

    height: 200

    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

    showArrow: true

    /*ColumnLayout {
        id: menuItems

        RowLayout {
            id: pedalSettings
            width: 30

            ColumnLayout {
                id: accidentalLabels
                height: 30

                Item {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                }

                StyledIconLabel {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    iconCode: IconCode.FLAT
                }

                StyledIconLabel {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    iconCode: IconCode.NATURAL
                }

                StyledIconLabel {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    iconCode: IconCode.SHARP
                }
            }

            Repeater {
                width: parent.width

                model: [
                    { buttonsId: DStringButtons, stringId: 0, stringName: "D" },
                    { buttonsId: CStringButtons, stringId: 1, stringName: "C" },
                    { buttonsId: BStringButtons, stringId: 2, stringName: "B" },
                    { buttonsId: EStringButtons, stringId: 3, stringName: "E" },
                    { buttonsId: FStringButtons, stringId: 4, stringName: "F" },
                    { buttonsId: GStringButtons, stringId: 5, stringName: "G" },
                    { buttonsId: AStringButtons, stringId: 6, stringName: "A" },
                ]

                delegate: ColumnLayout {
                    id: modelData["buttonsId"]
                    height: 30

                    Label {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        text: "D"
                    }

                    RoundedRadioButton {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        checked: false // function to get pedal state and check if flat

                        navigation.name: "DFlat"
                        navigation.panel: root.navigationPanel
                        navigation.row: 0
                        navigation.column: 0

                        onToggled: {
                            root.setPedal(D, FLAT)
                        }
                    }

                    RoundedRadioButton {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        checked: false // function to get pedal state and check if natural

                        navigation.name: "DNatural"
                        navigation.panel: root.navigationPanel
                        navigation.row: 1
                        navigation.column: 0

                        onToggled: {
                            root.setPedal(D, NATURAL)
                        }
                    }

                    RoundedRadioButton {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        checked: false // function to get pedal state and check if sharp

                        navigation.name: "DSharp"
                        navigation.panel: root.navigationPanel
                        navigation.row: 2
                        navigation.column: 0

                        onToggled: {
                            root.setPedal(D, SHARP)
                        }
                    }
                }

            }

            // Use repeater for this? see generalcomponentgallery
        }

        RowLayout {
            id: isDiagramButtons

            RoundedRadioButton {
                id: diagramButton

                checked: root.isDiagram
                text: qsTrc("notation", "Diagram")

                navigation.name: "diagramBox"
                navigation.panel: root.navigationPanel
                navigation.row: 0
                navigation.column: 0

                onToggled: {
                    root.setIsDiagram(true)
                }
            }

            RoundedRadioButton {
                id: textButton

                checked: !root.isDiagram
                text: qsTrc("notation", "Text")

                navigation.name: "textBox"
                navigation.panel: root.navigationPanel
                navigation.row: 0
                navigation.column: 1

                onToggled: {
                    root.setIsDiagram(true)
                }
            }
        }
    }

    // Label across top D C B | E F G A

    // Label down side flat natural sharp*/

    Label {
        text: model ? model.title : "no model"
        width: parent.width
    }
}
