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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Inspector 1.0
import "../../common"

Column {
    id: root

    property QtObject model: null

    objectName: "MMRestSettings"

    spacing: 12

    Column {
        spacing: 8

        height: childrenRect.height
        width: parent.width

        Item {
            height: childrenRect.height
            width: parent.width


            InspectorPropertyView {
                titleText: qsTrc("inspector", "Number visible")
                propertyItem: root.model ? root.model.isNumberVisible : null

                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                spacing: 8

                CheckBox {
                    id: numberVisibilityCheckBox

                    isIndeterminate: root.model ? root.model.isNumberVisible.isUndefined : false
                    checked: root.model && !isIndeterminate ? root.model.isNumberVisible.value : false
                    onClicked: { root.model.isNumberVisible.value = !checked }
                }
            }

            InspectorPropertyView {
                titleText: qsTrc("inspector", "Number position")
                propertyItem: root.model ? root.model.numberPosition : null

                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                spacing: 8

                IncrementalPropertyControl {
                    id: numberPositionControl

                    icon: IconCode.VERTICAL
                    enabled: root.model ? root.model.isEmpty || numberVisibilityCheckBox.checked : true
                    isIndeterminate: root.model ? root.model.numberPosition.isUndefined : false
                    currentValue: root.model ? root.model.numberPosition.value : 0

                    step: 0.5
                    decimals: 2
                    maxValue: 99.00
                    minValue: -99.00

                    onValueEdited: { root.model.numberPosition.value = newValue }
                }
            }
        }
    }
}
