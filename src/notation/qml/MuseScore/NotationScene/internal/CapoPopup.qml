/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore BVBA and others
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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.NotationScene 1.0

StyledPopupView {
    id: root

    property int navigationOrderStart: 0
    property int navigationOrderEnd: 0

    contentWidth: content.width
    contentHeight: content.height

    showArrow: false

    function updatePosition(pos, size) {
        root.x = pos.x + size.x + 12
        root.y = pos.y - root.contentHeight / 2
    }

    CapoSettingsModel {
        id: capoModel
    }

    Component.onCompleted: {
        capoModel.init()
    }

    ColumnLayout {
        id: content

        width: 294

        spacing: 12

        readonly property int columnsSpacing: 6

        StyledTextLabel {
            text: qsTrc("notation", "Capo")
            horizontalAlignment: Text.AlignLeft
        }

        FlatRadioButtonList {
            Layout.fillWidth: true

            spacing: content.columnsSpacing

            currentValue: capoModel.capoIsOn

            model: [
                { text: qsTrc("global", "On"), value: true },
                { text: qsTrc("global", "Off"), value: false }
            ]

            onToggled: function(newValue) {
                capoModel.capoIsOn = newValue
            }
        }

        StyledTextLabel {
            text: qsTrc("notation", "Fret")
            horizontalAlignment: Text.AlignLeft

            visible: capoModel.capoIsOn
        }

        IncrementalPropertyControl {
            id: fretPositionControl

            Layout.preferredWidth: parent.width / 2

            visible: capoModel.capoIsOn

            currentValue: capoModel.fretPosition
            decimals: 0
            step: 1
            minValue: 1
            maxValue: 12

            onValueEdited: function(newValue) {
                capoModel.fretPosition = newValue
            }
        }

        StyledTextLabel {
            text: qsTrc("notation", "Apply to")
            horizontalAlignment: Text.AlignLeft

            visible: capoModel.capoIsOn
        }

        GridLayout {
            id: applyToStringsGrid

            Layout.fillWidth: true

            visible: capoModel.capoIsOn

            columns: 2
            rows: Math.ceil(capoModel.strings.length / 2)
            columnSpacing: content.columnsSpacing
            rowSpacing: 12

            flow: GridLayout.TopToBottom

            Repeater {
                model: capoModel.strings

                Item {
                    height: toggleBtn.height
                    width: (content.width / 2 - content.columnsSpacing / applyToStringsGrid.columns)

                    ToggleButton {
                        id: toggleBtn

                        anchors.left: parent.left
                        anchors.top: parent.top

                        checked: modelData.applyCapo

                        onToggled: {
                            capoModel.toggleCapoForString(model.index)
                        }
                    }

                    StyledTextLabel {
                        anchors.left: toggleBtn.right
                        anchors.leftMargin: 4
                        anchors.verticalCenter: parent.verticalCenter

                        text: qsTrc("notation", "String") + " " + (model.index + 1)
                    }
                }
            }
        }

        CheckBox {
            id: specifyInstructionTextCheckBox

            Layout.fillWidth: true

            text: qsTrc("notation", "Manually specify instruction text")

            checked: capoModel.capoTextSpecifiedByUser

            onClicked: {
                capoModel.capoTextSpecifiedByUser = !checked
            }
        }

        TextInputField {
            Layout.fillWidth: true

            visible: specifyInstructionTextCheckBox.checked

            currentText: capoModel.userCapoText
            maximumLength: 40

            onTextEditingFinished: function(newTextValue) {
                capoModel.userCapoText = newTextValue
            }
        }

        StyledTextLabel {
            text: qsTrc("notation", "Position")
            horizontalAlignment: Text.AlignLeft
        }

        FlatRadioButtonList {
            Layout.fillWidth: true

            spacing: content.columnsSpacing

            currentValue: capoModel.capoPlacement
            model: capoModel.possibleCapoPlacements()

            onToggled: function(newValue) {
                capoModel.capoPlacement = newValue
            }
        }
    }
}
