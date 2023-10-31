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

        navigation.panel: root.navigationPanel
        navigation.row: directionSection.navigationRowEnd + 1
    }

    InspectorPropertyView {
        id: bendCurve
        titleText: qsTrc("inspector", "Click to add or remove points")

        enabled: true
        visible: true

        navigationPanel: root.navigationPanel
        navigationRowStart: showHold.navigation.row + 1

        BendGridCanvas {
            height: 200
            width: parent.width

            pointList: root.model ? root.model.bendCurve : null

            rowCount: 13
            columnCount: 13
            rowSpacing: 4
            columnSpacing: 3

            onCanvasChanged: {
                if (root.model) {
                    root.model.bendCurve = pointList
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
    }
}
