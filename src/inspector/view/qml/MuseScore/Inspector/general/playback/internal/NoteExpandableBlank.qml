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

    property int navigationRowEnd: contentItem.navigationRowEnd

    enabled: model ? !model.isEmpty : false

    title: model ? model.title : ""

    height: implicitHeight
    width: parent.width

    contentItemComponent: Column {
        property int navigationRowEnd: overrideDynamicsSection.navigation.row

        spacing: 12

        height: implicitHeight
        width: root.width

        Item {
            height: childrenRect.height
            width: root.width

            SpinBoxPropertyView {
                id: velocitySection
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                navigationPanel: root.navigation.panel
                navigationRowStart: root.navigation.row + 1
                navigationEnabled: root.navigation.enabled && root.enabled

                titleText: qsTrc("inspector", "Velocity")
                propertyItem: root.model ? root.model.velocity : null

                step: 1
                decimals: 0
                maxValue: 127
                minValue: -127
            }

            SpinBoxPropertyView {
                id: tuningsSection
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                navigationPanel: root.navigation.panel
                navigationRowStart: velocitySection.navigationRowEnd + 1
                navigationEnabled: root.navigation.enabled && root.enabled

                titleText: qsTrc("inspector", "Tunings (cents)")
                propertyItem: root.model ? root.model.tuning : null
            }
        }

        CheckBoxPropertyView {
            id: overrideDynamicsSection

            navigation.name: "Override dynamics"
            navigation.panel: root.navigation.panel
            navigation.column: root.navigation.column
            navigation.row: tuningsSection.navigationRowEnd + 1
            navigation.enabled: root.navigation.enabled && root.enabled

            text: qsTrc("inspector", "Override dynamics")
            propertyItem: root.model ? root.model.overrideDynamics : null
        }
    }
}
