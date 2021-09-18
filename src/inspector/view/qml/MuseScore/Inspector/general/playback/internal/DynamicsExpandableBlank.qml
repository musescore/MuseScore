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

        spacing: 12

        DropdownPropertyView {
            titleText: qsTrc("inspector", "Applies to")
            propertyItem: root.model ? root.model.scopeType : null

            navigation.name: "Dynamic Applies to"
            navigation.panel: root.navigation.panel
            navigation.column: root.navigation.column
            navigation.row: root.navigation.row + 1

            model: [
                { text: qsTrc("inspector", "Staff"), value: Dynamic.SCOPE_STAFF },
                { text: qsTrc("inspector", "Single instrument"), value: Dynamic.SCOPE_SINGLE_INSTRUMENT },
                { text: qsTrc("inspector", "All instruments"), value: Dynamic.SCOPE_ALL_INSTRUMENTS }
            ]
        }

        Item {
            height: childrenRect.height
            width: parent.width

            SpinBoxPropertyView {
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                navigation.name: "Velocity"
                navigation.panel: root.navigation.panel
                navigation.column: root.navigation.column
                navigation.row: root.navigation.row + 3

                titleText: qsTrc("inspector", "Velocity")
                propertyItem: root.model ? root.model.velocity : null

                step: 1
                decimals: 0
                maxValue: 127
                minValue: 0
            }

            SpinBoxPropertyView {
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                navigation.name: "Velocity change"
                navigation.panel: root.navigation.panel
                navigation.column: root.navigation.column
                navigation.row: root.navigation.row + 5

                titleText: qsTrc("inspector", "Velocity change")
                propertyItem: root.model ? root.model.velocityChange : null

                step: 1
                decimals: 0
                maxValue: 127
                minValue: -127
            }
        }

        FlatRadioButtonGroupPropertyView {
            titleText: qsTrc("inspector", "Change speed")
            propertyItem: root.model ? root.model.velocityChangeSpeed : null

            navigation.name: "Change speed Menu"
            navigation.panel: root.navigation.panel
            navigation.column: root.navigation.column
            navigation.row: root.navigation.row + 7

            model: [
                { text: "Slow", value: Dynamic.VELOCITY_CHANGE_SPEED_SLOW },
                { text: "Normal", value: Dynamic.VELOCITY_CHANGE_SPEED_NORMAL },
                { text: "Fast", value: Dynamic.VELOCITY_CHANGE_SPEED_FAST }
            ]
        }
    }
}
