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
import QtQuick.Controls 2.15

import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0

import "../../../common"

FocusableItem {
    id: root

    property QtObject model: null

    implicitHeight: contentColumn.height
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        Item {
            height: childrenRect.height
            width: parent.width

            SpinBoxPropertyView {
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                titleText: qsTrc("inspector", "Scale")
                propertyItem: root.model ? root.model.scale : null

                measureUnitsSymbol: "%"
                step: 1
                decimals: 0
                maxValue: 300
                minValue: 1
            }

            SpinBoxPropertyView {
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                titleText: qsTrc("inspector", "Strings")
                propertyItem: root.model ? root.model.stringsCount : null

                step: 1
                decimals: 0
                maxValue: 12
                minValue: 4
            }
        }

        Item {
            height: childrenRect.height
            width: parent.width

            SpinBoxPropertyView {
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                titleText: qsTrc("inspector", "Visible frets")
                propertyItem: root.model ? root.model.fretsCount : null

                step: 1
                decimals: 0
                maxValue: 6
                minValue: 3
            }

            SpinBoxPropertyView {
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                titleText: qsTrc("inspector", "Starting fret number")
                propertyItem: root.model ? root.model : null

                step: 1
                decimals: 0
                maxValue: 12
                minValue: 1
            }
        }

        PlacementSection {
            titleText: qsTrc("inspector", "Placement on staff")
            propertyItem: root.model ? root.model.placement : null
        }

        CheckBox {
            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 2

            isIndeterminate: root.model ? root.model.isNutVisible.isUndefined : false
            checked: root.model && !isIndeterminate ? root.model.isNutVisible.value : false
            text: qsTrc("inspector", "Show nut")

            onClicked: { root.model.isNutVisible.value = !checked }
        }
    }
}
