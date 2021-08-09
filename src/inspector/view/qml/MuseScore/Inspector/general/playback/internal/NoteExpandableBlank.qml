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

    height: implicitHeight
    width: parent.width

    contentItemComponent: Column {
        spacing: 16

        height: implicitHeight
        width: root.width

        Item {
            height: childrenRect.height
            width: root.width

            InspectorPropertyView {
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                navigation.name: "VelocityMenu"
                navigation.panel: root.navigation.panel
                navigation.column: root.navigation.column
                navigation.row: root.navigation.row + 1

                titleText: qsTrc("inspector", "Velocity")
                propertyItem: root.model ? root.model.velocity : null

                IncrementalPropertyControl {
                    id: velocityControl

                    navigation.name: "VelocityValue"
                    navigation.panel: root.navigation.panel
                    navigation.column: root.navigation.column
                    navigation.row: root.navigation.row + 2

                    step: 1
                    decimals: 0
                    maxValue: 127
                    minValue: -127

                    isIndeterminate: root.model ? root.model.velocity.isUndefined : false
                    currentValue: root.model ? root.model.velocity.value : 0

                    onValueEdited: { root.model.velocity.value = newValue }
                }
            }

            InspectorPropertyView {
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                navigation.name: "TuningsMenu"
                navigation.panel: root.navigation.panel
                navigation.column: root.navigation.column
                navigation.row: root.navigation.row + 3

                titleText: qsTrc("inspector", "Tunings (cents)")
                propertyItem: root.model ? root.model.tuning : null

                IncrementalPropertyControl {
                    navigation.name: "TuningsValue"
                    navigation.panel: root.navigation.panel
                    navigation.column: root.navigation.column
                    navigation.row: root.navigation.row + 4

                    isIndeterminate: root.model ? root.model.tuning.isUndefined : false
                    currentValue: root.model ? root.model.tuning.value : 0

                    onValueEdited: { root.model.tuning.value = newValue }
                }
            }
        }

        CheckBox {
            navigation.name: "Override dynamics"
            navigation.panel: root.navigation.panel
            navigation.column: root.navigation.column
            navigation.row: root.navigation.row + 5

            text: qsTrc("inspector", "Override dynamics")

            isIndeterminate: root.model ? root.model.overrideDynamics.isUndefined : false
            checked: root.model && !isIndeterminate ? root.model.overrideDynamics.value : false

            onClicked: { root.model.overrideDynamics.value = !checked }
        }
    }
}
