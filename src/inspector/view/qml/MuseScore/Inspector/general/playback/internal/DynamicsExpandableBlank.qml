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
import QtQuick.Controls 2.0
import MuseScore.UiComponents 1.0
import MuseScore.Inspector 1.0
import "../../../common"

ExpandableBlank {
    id: root

    property QtObject model: null

    enabled: model ? !model.isEmpty : false

    title: model ? model.title : ""

    width: parent.width

    contentItemComponent: Column {
        id: contentLayout

        height: implicitHeight
        width: root.width

        spacing: 16

        InspectorPropertyView {
            titleText: qsTrc("inspector", "Applies to")
            propertyItem: root.model ? root.model.scopeType : null

            navigation.name: "Dynamic Applies Menu"
            navigation.panel: root.navigation.panel
            navigation.column: root.navigation.column
            navigation.row: root.navigation.row + 1

            Dropdown {
                id: applies

                width: parent.width

                navigation.name: "Dynamic Applies Value"
                navigation.panel: root.navigation.panel
                navigation.column: root.navigation.column
                navigation.row: root.navigation.row + 2

                model: [
                    { text: qsTrc("inspector", "Staff"), value: Dynamic.SCOPE_STAFF },
                    { text: qsTrc("inspector", "Single instrument"), value: Dynamic.SCOPE_SINGLE_INSTRUMENT },
                    { text: qsTrc("inspector", "All instruments"), value: Dynamic.SCOPE_ALL_INSTRUMENTS }
                ]

                currentIndex: root.model && !root.model.scopeType.isUndefined ? applies.indexOfValue(root.model.scopeType.value) : -1

                onCurrentValueChanged: {
                    root.model.scopeType.value = applies.currentValue
                }
            }
        }

        Item {
            height: childrenRect.height
            width: parent.width

            InspectorPropertyView {
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                navigation.name: "Velocity Menu"
                navigation.panel: root.navigation.panel
                navigation.column: root.navigation.column
                navigation.row: root.navigation.row + 3

                titleText: qsTrc("inspector", "Velocity")
                propertyItem: model ? model.velocity : null

                IncrementalPropertyControl {
                    id: velocityControl
                    iconMode: iconModeEnum.hidden

                    navigation.name: "Velocity Value"
                    navigation.panel: root.navigation.panel
                    navigation.column: root.navigation.column
                    navigation.row: root.navigation.row + 4

                    step: 1
                    decimals: 0
                    maxValue: 127
                    minValue: 0
                    validator: IntInputValidator {
                        top: velocityControl.maxValue
                        bottom: velocityControl.minValue
                    }

                    isIndeterminate: model ? model.velocity.isUndefined : false
                    currentValue: model ? model.velocity.value : 0

                    onValueEdited: { model.velocity.value = newValue }
                }
            }

            InspectorPropertyView {
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                navigation.name: "Velocity change Menu"
                navigation.panel: root.navigation.panel
                navigation.column: root.navigation.column
                navigation.row: root.navigation.row + 5

                titleText: qsTrc("inspector", "Velocity change")
                propertyItem: model ? model.velocityChange : null

                IncrementalPropertyControl {
                    id: velocityChangeControl
                    iconMode: iconModeEnum.hidden

                    navigation.name: "Velocity change Value"
                    navigation.panel: root.navigation.panel
                    navigation.column: root.navigation.column
                    navigation.row: root.navigation.row + 6

                    enabled: model ? model.velocityChange.isEnabled : false

                    step: 1
                    decimals: 0
                    maxValue: 127
                    minValue: -127
                    validator: IntInputValidator {
                        top: velocityChangeControl.maxValue
                        bottom: velocityChangeControl.minValue
                    }

                    isIndeterminate: model && enabled ? model.velocityChange.isUndefined : false
                    currentValue: model ? model.velocityChange.value : 0

                    onValueEdited: { model.velocityChange.value = newValue }
                }
            }
        }

        InspectorPropertyView {
            titleText: qsTrc("inspector", "Change speed")

            navigation.name: "Change speed Menu"
            navigation.panel: root.navigation.panel
            navigation.column: root.navigation.column
            navigation.row: root.navigation.row + 7

            propertyItem: root.model ? root.model.velocityChangeSpeed : null

            RadioButtonGroup {
                id: radioButtonList

                height: 30
                width: parent.width

                model: [
                    { textRole: "Slow", valueRole: Dynamic.VELOCITY_CHANGE_SPEED_SLOW },
                    { textRole: "Normal", valueRole: Dynamic.VELOCITY_CHANGE_SPEED_NORMAL },
                    { textRole: "Fast", valueRole: Dynamic.VELOCITY_CHANGE_SPEED_FAST }
                ]

                delegate: FlatRadioButton {
                    id: radioButtonDelegate

                    navigation.name: "Change speed Value " + model.index
                    navigation.panel: root.navigation.panel
                    navigation.column: root.navigation.column
                    navigation.row: root.navigation.row + 8 + model.index

                    ButtonGroup.group: radioButtonList.radioButtonGroup

                    checked: root.model && !root.model.velocityChangeSpeed.isUndefined ? root.model.velocityChangeSpeed.value === modelData["valueRole"]
                                                                                       : false
                    onToggled: {
                        root.model.velocityChangeSpeed.value = modelData["valueRole"]
                    }

                    StyledTextLabel {
                        text: modelData["textRole"]

                        elide: Text.ElideRight
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                    }
                }
            }
        }
    }
}
