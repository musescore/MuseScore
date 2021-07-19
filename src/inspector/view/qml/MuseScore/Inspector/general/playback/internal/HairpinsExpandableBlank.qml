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
        height: implicitHeight
        width: parent.width

        spacing: 12

        InspectorPropertyView {
            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 2

            navigation.name: "Velocity change Menu"
            navigation.panel: root.navigation.panel
            navigation.column: root.navigation.column
            navigation.row: root.navigation.row + 1

            titleText: qsTrc("inspector", "Velocity change")
            propertyItem: model ? model.velocityChange : null

            IncrementalPropertyControl {
                id: velocityChangeControl
                iconMode: iconModeEnum.hidden

                navigation.name: "Velocity change Value"
                navigation.panel: root.navigation.panel
                navigation.column: root.navigation.column
                navigation.row: root.navigation.row + 2

                step: 1
                decimals: 0
                maxValue: 127
                minValue: 0
                validator: IntInputValidator {
                    top: velocityChangeControl.maxValue
                    bottom: velocityChangeControl.minValue
                }

                isIndeterminate: model ? model.velocityChange.isUndefined : false
                currentValue: model ? model.velocityChange.value : 0

                onValueEdited: { model.velocityChange.value = newValue }
            }
        }

        InspectorPropertyView {
            titleText: qsTrc("inspector", "Changes in dynamics range")

            navigation.name: "Changes in dynamics range Menu"
            navigation.panel: root.navigation.panel
            navigation.column: root.navigation.column
            navigation.row: root.navigation.row + 3

            propertyItem: root.model ? root.model.velocityChangeType : null

            Dropdown {
                id: dranges

                width: parent.width

                navigation.name: "Changes in dynamics range Value"
                navigation.panel: root.navigation.panel
                navigation.column: root.navigation.column
                navigation.row: root.navigation.row + 4

                model: [
                    { text: qsTrc("inspector", "Linear (default)"), value: Hairpin.VELOCITY_EASING_LINEAR },
                    { text: qsTrc("inspector", "Exponential"), value: Hairpin.VELOCITY_EASING_EXPONENTIAL },
                    { text: qsTrc("inspector", "Ease-in"), value: Hairpin.VELOCITY_EASING_IN },
                    { text: qsTrc("inspector", "Ease-out"), value: Hairpin.VELOCITY_EASING_OUT },
                    { text: qsTrc("inspector", "Ease-in and out"), value: Hairpin.VELOCITY_EASING_IN_OUT }
                ]

                currentIndex: root.model && !root.model.velocityChangeType.isUndefined ? dranges.indexOfValue(root.model.velocityChangeType.value) : -1

                onCurrentValueChanged: {
                    root.model.velocityChangeType.value = dranges.currentValue
                }
            }
        }
    }
}

