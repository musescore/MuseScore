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

    objectName: "TimeSignatureSettings"

    spacing: 12

    InspectorPropertyView {
        height: childrenRect.height

        titleText: qsTrc("inspector", "Scale")
        propertyItem: root.model ? root.model.horizontalScale : null

        Item {
            height: childrenRect.height
            width: parent.width

            IncrementalPropertyControl {
                id: horizontalScaleControl

                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                icon: IconCode.HORIZONTAL
                isIndeterminate: root.model ? root.model.horizontalScale.isUndefined : false
                currentValue: root.model ? root.model.horizontalScale.value : 0

                measureUnitsSymbol: "%"
                step: 1
                decimals: 0
                maxValue: 300
                minValue: 1
                validator: IntInputValidator {
                    top: horizontalScaleControl.maxValue
                    bottom: horizontalScaleControl.minValue
                }

                onValueEdited: { root.model.horizontalScale.value = newValue }
            }

            IncrementalPropertyControl {
                id: verticalScaleControl

                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                icon: IconCode.VERTICAL
                isIndeterminate: root.model ? root.model.verticalScale.isUndefined : false
                currentValue: root.model ? root.model.verticalScale.value : 0

                measureUnitsSymbol: "%"
                step: 1
                decimals: 0
                maxValue: 300
                minValue: 1
                validator: IntInputValidator {
                    top: verticalScaleControl.maxValue
                    bottom: verticalScaleControl.minValue
                }

                onValueEdited: { root.model.verticalScale.value = newValue }
            }
        }
    }

    CheckBox {
        isIndeterminate: root.model ? root.model.shouldShowCourtesy.isUndefined : false
        checked: root.model && !isIndeterminate ? root.model.shouldShowCourtesy.value : false
        text: qsTrc("inspector", "Show courtesy time signature on previous system")

        onClicked: { root.model.shouldShowCourtesy.value = !checked }
    }

    FlatButton {
        width: parent.width

        text: qsTrc("inspector", "Change time signature")

        onClicked: {
            if (root.model) {
                root.model.showTimeSignatureProperties()
            }
        }
    }
}
