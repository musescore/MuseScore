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
import "../../common"

Column {
    id: root

    property QtObject model: null

    objectName: "BendSettings"

    spacing: 12

    InspectorPropertyView {
        titleText: qsTrc("inspector", "Bend type")
        propertyItem: root.model ? root.model.bendType : null

        Dropdown {
            id: btypes

            width: parent.width

            model: [
                { text: qsTrc("inspector", "Bend"), value: BendTypes.TYPE_BEND },
                { text: qsTrc("inspector", "Bend/Release"), value: BendTypes.TYPE_BEND_RELEASE },
                { text: qsTrc("inspector", "Bend/Release/Bend"), value: BendTypes.TYPE_BEND_RELEASE_BEND },
                { text: qsTrc("inspector", "Prebend"), value: BendTypes.TYPE_PREBEND },
                { text: qsTrc("inspector", "Prebend/Release"), value: BendTypes.TYPE_PREBEND_RELEASE },
                { text: qsTrc("inspector", "Custom"), value: BendTypes.TYPE_CUSTOM }
            ]

            currentIndex: root.model && !root.model.bendType.isUndefined ? btypes.indexOfValue(root.model.bendType.value) : -1

            onCurrentValueChanged: {
                if (currentIndex === -1) {
                    return
                }

                root.model.bendType.value = btypes.currentValue
            }
        }
    }

    InspectorPropertyView {
        titleText: qsTrc("inspector", "Click to add or remove points")
        propertyItem: root.model ? root.model.bendCurve : null

        GridCanvas {
            height: 200
            width: parent.width

            pointList: root.model && root.model.bendCurve.isEnabled ? root.model.bendCurve.value : undefined

            rowCount: 13
            columnCount: 13
            rowSpacing: 4
            columnSpacing: 3

            onCanvasChanged: {
                if (root.model) {
                    root.model.bendCurve.value = pointList
                }
            }
        }
    }

    InspectorPropertyView {
        titleText: qsTrc("inspector", "Line thickness")
        propertyItem: root.model ? root.model.lineThickness : null

        IncrementalPropertyControl {
            isIndeterminate: root.model ? root.model.lineThickness.isUndefined : false
            currentValue: root.model ? root.model.lineThickness.value : 0
            iconMode: iconModeEnum.hidden

            maxValue: 10
            minValue: 0.1
            step: 0.1
            decimals: 2

            onValueEdited: { root.model.lineThickness.value = newValue }
        }
    }
}
