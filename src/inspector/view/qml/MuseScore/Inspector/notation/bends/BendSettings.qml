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

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "BendSettings"

    spacing: 12

    function focusOnFirst() {
        bendTypeSection.focusOnFirst()
    }

    DirectionSection {
        id: directionSection

        propertyItem: root.model ? root.model.bendDirection : null

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart + 1
    }

    PropertyCheckBox {
        id: showHold
        visible: root.model ? root.model.isShowHoldLineAvailable : false

        text: qsTrc("inspector", "Show hold line")
        propertyItem: root.model ? root.model.showHoldLine : null
    }

    /*DropdownPropertyView {
        id: bendTypeSection
        titleText: qsTrc("inspector", "Bend type")
        propertyItem: root.model ? root.model.bendType : null

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart + 1

        model: [
            { text: qsTrc("inspector", "Bend"), value: BendTypes.TYPE_BEND },
            { text: qsTrc("inspector", "Bend/Release"), value: BendTypes.TYPE_BEND_RELEASE },
            { text: qsTrc("inspector", "Bend/Release/Bend"), value: BendTypes.TYPE_BEND_RELEASE_BEND },
            { text: qsTrc("inspector", "Prebend"), value: BendTypes.TYPE_PREBEND },
            { text: qsTrc("inspector", "Prebend/Release"), value: BendTypes.TYPE_PREBEND_RELEASE },
            { text: qsTrc("inspector", "Custom"), value: BendTypes.TYPE_CUSTOM }
        ]
    }

    InspectorPropertyView {
        id: bendCurve
        titleText: qsTrc("inspector", "Click to add or remove points")
        propertyItem: root.model ? root.model.bendCurve : null

        navigationPanel: root.navigationPanel
        navigationRowStart: bendTypeSection.navigationRowEnd + 1

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

    SpinBoxPropertyView {
        titleText: qsTrc("inspector", "Line thickness")
        propertyItem: root.model ? root.model.lineThickness : null

        maxValue: 10
        minValue: 0.1
        step: 0.1
        decimals: 2

        navigationPanel: root.navigationPanel
        navigationRowStart: bendCurve.navigationRowEnd + 1
    }*/
}
