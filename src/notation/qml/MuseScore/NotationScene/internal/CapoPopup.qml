/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2023 MuseScore Limited
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.NotationScene 1.0

StyledPopupView {
    id: root

    property alias notationViewNavigationSection: capoSettingsNavPanel.section
    property alias navigationOrderStart: capoSettingsNavPanel.order
    readonly property alias navigationOrderEnd: capoSettingsNavPanel.order


    contentWidth: content.width
    contentHeight: content.height

    showArrow: false

    signal elementRectChanged(var elementRect)

    function updatePosition() {
        var h = Math.max(root.contentHeight, capoModel.capoIsOn ? 360 : 160)
        root.x = root.parent.width + 12
        root.y = (root.parent.y + root.parent.height / 2) - root.parent.y - h / 2
    }

    ColumnLayout {
        id: content

        readonly property int columnsSpacing: 6

        width: 294

        spacing: 12

        CapoSettingsModel {
            id: capoModel

            onItemRectChanged: function(rect) {
                root.elementRectChanged(rect)
            }
        }

        Component.onCompleted: {
            capoModel.init()
        }

        NavigationPanel {
            id: capoSettingsNavPanel
            name: "CapoSettings"
            direction: NavigationPanel.Vertical
            accessible.name: qsTrc("notation", "Capo settings")

            onNavigationEvent: function(event) {
                if (event.type === NavigationEvent.Escape) {
                    root.close()
                }
            }
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
                //: as opposed to Off
                { text: qsTrc("global", "On"), value: true },
                //: as opposed to On
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
