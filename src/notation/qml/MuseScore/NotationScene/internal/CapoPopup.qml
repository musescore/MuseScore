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

    property NavigationSection notationViewNavigationSection: null
    property int navigationOrderStart: 0
    property int navigationOrderEnd: capoSettingsNavPanel.order

    contentWidth: content.width
    contentHeight: content.height

    showArrow: false

    function updatePosition(pos, size) {
        var h = Math.max(root.contentHeight, capoModel.capoIsOn ? 360 : 160)
        root.x = pos.x + size.x + 12
        root.y = pos.y - h / 2
    }

    CapoSettingsModel {
        id: capoModel

        onItemRectChanged: function(rect) {
            updatePosition(Qt.point(rect.x, rect.y), Qt.point(rect.width, rect.height))
        }
    }

    Component.onCompleted: {
        capoModel.init()
    }

    ColumnLayout {
        id: content

        width: 294

        spacing: 12

        readonly property int columnsSpacing: 6

        NavigationPanel {
            id: capoSettingsNavPanel
            name: "CapoSettings"
            direction: NavigationPanel.Vertical
            section: root.notationViewNavigationSection
            order: root.navigationOrderStart
            accessible.name: qsTrc("notation", "Capo settings")
        }

        StyledTextLabel {
            id: titleLabel

            text: qsTrc("notation", "Capo")
            horizontalAlignment: Text.AlignLeft
        }

        FlatRadioButtonList {
            id: capoOnOffButtons

            Layout.fillWidth: true

            spacing: content.columnsSpacing

            navigationPanel: capoSettingsNavPanel
            navigationRowStart: root.navigationOrderStart
            accessibleName: titleLabel.text

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
            id: fretLabel

            text: qsTrc("notation", "Fret")
            horizontalAlignment: Text.AlignLeft

            visible: capoModel.capoIsOn
        }

        IncrementalPropertyControl {
            id: fretControl

            Layout.preferredWidth: parent.width / 2 - content.columnsSpacing / 2

            visible: capoModel.capoIsOn

            navigation.name: "FretControl"
            navigation.panel: capoSettingsNavPanel
            navigation.row: capoOnOffButtons.navigationRowEnd + 1
            navigation.accessible.name: fretLabel.text + " " + fretControl.currentValue

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
            id: applyToLabel

            text: qsTrc("notation", "Apply to")
            horizontalAlignment: Text.AlignLeft

            visible: capoModel.capoIsOn && repeaterStrings.count > 0
        }

        GridLayout {
            id: applyToStringsGrid

            readonly property int navigationRowStart: fretControl.navigation.row + 1
            readonly property int navigationRowEnd: applyToStringsGrid.navigationRowStart + repeaterStrings.count

            Layout.fillWidth: true

            visible: capoModel.capoIsOn

            columns: 2
            rows: Math.ceil(capoModel.strings.length / 2)
            columnSpacing: content.columnsSpacing
            rowSpacing: 12

            flow: GridLayout.TopToBottom

            Repeater {
                id: repeaterStrings

                model: capoModel.strings

                Item {
                    height: toggleBtn.height
                    width: (content.width / 2 - content.columnsSpacing / applyToStringsGrid.columns)

                    ToggleButton {
                        id: toggleBtn

                        anchors.left: parent.left
                        anchors.top: parent.top

                        navigation.name: "String" + model.index + "Control"
                        navigation.panel: capoSettingsNavPanel
                        navigation.row: applyToStringsGrid.navigationRowStart + model.index
                        navigation.accessible.name: applyToLabel.text + " " + stringLabel.text

                        checked: modelData.applyCapo

                        onToggled: {
                            capoModel.toggleCapoForString(model.index)
                        }
                    }

                    StyledTextLabel {
                        id: stringLabel

                        anchors.left: toggleBtn.right
                        anchors.leftMargin: 4
                        anchors.verticalCenter: parent.verticalCenter

                        text: qsTrc("notation", "String %1").arg(model.index + 1)
                    }
                }
            }
        }

        CheckBox {
            id: specifyInstructionTextCheckBox

            Layout.fillWidth: true

            navigation.panel: capoSettingsNavPanel
            navigation.row: applyToStringsGrid.navigationRowEnd + 1

            text: qsTrc("notation", "Manually specify instruction text")

            checked: capoModel.capoTextSpecifiedByUser

            onClicked: {
                capoModel.capoTextSpecifiedByUser = !checked
            }
        }

        TextInputField {
            id: capoTextField

            Layout.fillWidth: true

            visible: specifyInstructionTextCheckBox.checked

            navigation.panel: capoSettingsNavPanel
            navigation.row: specifyInstructionTextCheckBox.navigation.row + 1

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

            navigationPanel: capoSettingsNavPanel
            navigationRowStart: capoTextField.navigation.row + 1
            accessibleName: qsTrc("notation", "Capo position")

            currentValue: capoModel.capoPlacement
            model: capoModel.possibleCapoPlacements()

            onToggled: function(newValue) {
                capoModel.capoPlacement = newValue
            }
        }
    }
}
