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
import QtQuick.Layouts 1.15

import MuseScore.NotationScene 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0

StyleDialogPage {
    id: root

    BeamsPageModel {
        id: beamsPageModel
    }

    ItemWithTitle {
        title: qsTrc("notation", "Beam distance")

        RadioButtonGroup {
            width: 224
            height: 70
            spacing: 12

            model: [
                { iconCode: IconCode.USE_WIDE_BEAMS_REGULAR, text: qsTrc("notation", "Regular"), value: false },
                { iconCode: IconCode.USE_WIDE_BEAMS_WIDE, text: qsTrc("notation", "Wide"), value: true }
            ]

            delegate: FlatRadioButton {
                width: 106
                height: 70

                checked: modelData.value === beamsPageModel.useWideBeams.value

                Column {
                    anchors.centerIn: parent
                    height: childrenRect.height
                    spacing: 8

                    StyledIconLabel {
                        anchors.horizontalCenter: parent.horizontalCenter
                        iconCode: modelData.iconCode
                        font.pixelSize: 28
                    }

                    StyledTextLabel {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: modelData.text
                    }
                }

                onToggled: {
                    beamsPageModel.useWideBeams.value = modelData.value
                }
            }
        }
    }

    ItemWithTitle {
        title: qsTrc("notation", "Beam thickness")

        IncrementalPropertyControl {
            width: 106

            currentValue: beamsPageModel.beamWidth.value

            minValue: 0
            maxValue: 99
            step: 0.01
            decimals: 2
            measureUnitsSymbol: qsTrc("global", "sp")

            onValueEdited: function(newValue) {
                beamsPageModel.beamWidth.value = newValue
            }
        }
    }

    ItemWithTitle {
        title: qsTrc("notation", "Broken beam minimum length")

        IncrementalPropertyControl {
            width: 106

            currentValue: beamsPageModel.beamMinLen.value

            minValue: 0
            maxValue: 99
            step: 0.05
            decimals: 2
            measureUnitsSymbol: qsTrc("global", "sp")

            onValueEdited: function(newValue) {
                beamsPageModel.beamMinLen.value = newValue
            }
        }
    }

    CheckBox {
        width: parent.width
        text: qsTrc("notation", "Flatten all beams")
        checked: beamsPageModel.beamNoSlope.value
        onClicked: {
            beamsPageModel.beamNoSlope.value = !checked
        }
    }

    StyledGroupBox {
        width: parent.width
        height: Math.max(120, implicitHeight)

        title: qsTrc("notation", "Beam style")

        RowLayout {
            anchors.fill: parent
            spacing: 12

            RadioButtonGroup {
                Layout.fillWidth: true

                spacing: 12
                orientation: ListView.Vertical

                model: [
                    { title: qsTrc("notation", "Draw inner stems through beams"), value: false },
                    { title: qsTrc("notation", "Draw inner stems to nearest beam (“French” style)"), value: true }
                ]

                delegate: RoundedRadioButton {
                    width: ListView.view.width
                    text: modelData.title
                    checked: modelData.value === beamsPageModel.frenchStyleBeams.value
                    onToggled: {
                        beamsPageModel.frenchStyleBeams.value = modelData.value
                    }
                }
            }

            Rectangle {
                id: frenchStyleBeamFrame
                width: 200
                Layout.fillHeight: true
                Layout.alignment: Qt.AlignRight | Qt.AlignVCenter

                color: "#ffffff"
                border.color: ui.theme.strokeColor
                radius: ui.theme.borderWidth

                Image {
                    width: Math.min(62, parent.width)
                    mipmap: true
                    sourceSize.width: 240
                    sourceSize.height: 240
                    anchors.centerIn: parent
                    fillMode: Image.PreserveAspectFit
                    source: beamsPageModel.frenchStyleBeams.value ? "beam_style_french.svg" : "beam_style_regular.svg"
                }
            }
        }
    }
}
