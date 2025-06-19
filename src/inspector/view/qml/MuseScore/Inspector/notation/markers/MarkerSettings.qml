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
import QtQuick.Controls 2.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../../common"

Column {
    id: root

    property QtObject model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "MarkerSettings"

    spacing: 12

    function focusOnFirst() {
        labelSection.focusOnFirst()
    }

    StyledTextLabel {
        width: parent.width
        text: qsTrc("inspector", "Marker type:") + " " + markerTypeToString(root.model ? root.model.type : null)
        horizontalAlignment: Text.AlignLeft

        function markerTypeToString(type) {
            if (!type)
                return ""

            if (type.isUndefined)
                return "--"

            switch (type.value) {
            case MarkerTypes.TYPE_SEGNO: return qsTrc("inspector", "Segno");
            case MarkerTypes.TYPE_VARSEGNO: return qsTrc("inspector", "Segno variation")
            case MarkerTypes.TYPE_CODA: return qsTrc("inspector", "Coda")
            case MarkerTypes.TYPE_VARCODA: return qsTrc("inspector", "Varied coda")
            case MarkerTypes.TYPE_CODETTA: return qsTrc("inspector", "Codetta")
            case MarkerTypes.TYPE_FINE: return qsTrc("inspector", "Fine")
            case MarkerTypes.TYPE_TOCODA: return qsTrc("inspector", "To Coda")
            case MarkerTypes.TYPE_USER: return qsTrc("inspector", "Custom")
            }
        }
    }

    TextSection {
        id: labelSection
        titleText: qsTrc("inspector", "Label")
        propertyItem: root.model ? root.model.label : null

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart
    }

    SpinBoxPropertyView {
        id: symbolSize

        anchors.left: parent.left

        navigationName: "Symbol size"
        navigationPanel: root.navigationPanel
        navigationRowStart: labelSection.navigationRowEnd + 1

        titleText: qsTrc("inspector", "Symbol size")
        measureUnitsSymbol: "pt"
        propertyItem: root.model ? root.model.symbolSize : null

        decimals: 0
        step: 1
        minValue: 0
        maxValue: 1000
    }

    CheckBoxPropertyView {
        id: alignSymbolCheckbox
        titleText: qsTrc("inspector", "Align symbol with barline")
        propertyItem: root.model ? root.model.centerOnSymbol : null

        navigationPanel: root.navigationPanel
        navigationRowStart: symbolSize.navigationRowEnd + 1
    }

    FlatRadioButtonGroupPropertyView {
        id: alignmentButtonList
        titleText: qsTrc("inspector", "Alignment to barline")
        propertyItem: root.model ? root.model.position : null

        enabled: root.model ? !root.model.centerOnSymbol.value : false

        navigationPanel: root.navigationPanel
        navigationRowStart: alignSymbolCheckbox.navigationRowEnd + 1

        requestIconFontSize: 16
        requestWidth: 98

        model: [
            { iconCode: IconCode.ALIGN_LEFT, value: 0},
            { iconCode: IconCode.ALIGN_HORIZONTAL_CENTER, value: 2},
            { iconCode: IconCode.ALIGN_RIGHT, value: 1 }
        ]
    }
}
