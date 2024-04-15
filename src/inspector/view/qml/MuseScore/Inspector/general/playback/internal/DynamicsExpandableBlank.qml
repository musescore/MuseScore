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

import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../../../common"

ExpandableBlank {
    id: root

    property QtObject model: null

    property int navigationRowEnd: contentItem.navigationRowEnd

    enabled: model ? !model.isEmpty : false

    title: model ? model.title : ""

    width: parent.width

    contentItemComponent: Column {
        id: contentLayout

        property int navigationRowEnd: changeSpeed.navigationRowEnd

        height: implicitHeight
        width: root.width

        spacing: 12

        Item {
            height: childrenRect.height
            width: parent.width

            SpinBoxPropertyView {
                id: velocitySection
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                navigationName: "Velocity"
                navigationPanel: root.navigation.panel
                navigationRowStart: root.navigation.row + 1

                titleText: qsTrc("inspector", "Velocity")
                propertyItem: root.model ? root.model.velocity : null

                step: 1
                decimals: 0
                maxValue: 127
                minValue: 0
            }

            SpinBoxPropertyView {
                id: velocityChangeSection
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                navigationName: "Velocity change"
                navigationPanel: root.navigation.panel
                navigationRowStart: velocitySection.navigationRowEnd + 1

                titleText: qsTrc("inspector", "Velocity change")
                propertyItem: root.model ? root.model.velocityChange : null

                step: 1
                decimals: 0
                maxValue: 127
                minValue: -127
            }
        }

        FlatRadioButtonGroupPropertyView {
            id: changeSpeed
            titleText: qsTrc("inspector", "Change speed")
            propertyItem: root.model ? root.model.velocityChangeSpeed : null

            navigationName: "Change speed Menu"
            navigationPanel: root.navigation.panel
            navigationRowStart: velocityChangeSection.navigationRowEnd + 1

            model: [
                { text: qsTrc("inspector", "Slow", "velocity change speed"), value: Dynamic.VELOCITY_CHANGE_SPEED_SLOW },
                { text: qsTrc("inspector", "Normal", "velocity change speed"), value: Dynamic.VELOCITY_CHANGE_SPEED_NORMAL },
                { text: qsTrc("inspector", "Fast", "velocity change speed"), value: Dynamic.VELOCITY_CHANGE_SPEED_FAST }
            ]
        }
    }
}
