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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../../common"
import "internal"

Column {
    id: root

    property QtObject model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    property bool isLaissezVib: model ? model.isLaissezVib : false

    objectName: "SlurAndTieSettings"

    spacing: 12

    function focusOnFirst() {
        styleSection.focusOnFirst()
    }

    LineStyleSection {
        id: styleSection

        visible: root.model ? root.model.isLineStyleAvailable : false
        lineStyle: root.model ? root.model.lineStyle : null
        possibleLineStyles: root.model ? root.model.possibleLineStyles() : []

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart + 1

        lineStyleTitleText: qsTrc("inspector", "Style")
    }

    SeparatorLine {
        visible: root.model ? root.model.isLineStyleAvailable : false
        anchors.margins: -12
    }

    PlacementSection {
        id: placementSection

        propertyItem: root.model ? root.model.direction : null

        navigationPanel: root.navigationPanel
        navigationRowStart: styleSection.navigationRowEnd + 1

        //! NOTE: Slur/tie uses the direction property,
        // but for convenience we will display it in the placement section
        model: [
            { text: qsTrc("inspector", "Auto"), value: DirectionTypes.VERTICAL_AUTO },
            { text: qsTrc("inspector", "Above"), value: DirectionTypes.VERTICAL_UP },
            { text: qsTrc("inspector", "Below"), value: DirectionTypes.VERTICAL_DOWN }
        ]
    }

    FlatRadioButtonGroupPropertyView {
        id: tiePlacementSection

        property bool isLaissezVib: false

        visible: root.model ? root.model.isTiePlacementAvailable : false
        propertyItem: root.model ? root.model.tiePlacement : null
        titleText: root.isLaissezVib ? qsTrc("inspector", "Laissez vibrer placement") : qsTrc("inspector", "Tie placement")

        navigationPanel: root.navigationPanel
        navigationRowStart: placementSection.navigationRowEnd + 1

        model: [
            { text: qsTrc("inspector", "Auto"), value: SlurTieTypes.TIE_PLACEMENT_AUTO, title: qsTrc("inspector", "Auto")  },
            { iconCode: root.isLaissezVib ? IconCode.LV_INSIDE : IconCode.TIE_INSIDE, value: SlurTieTypes.TIE_PLACEMENT_INSIDE, title: qsTrc("inspector", "Inside") },
            { iconCode: root.isLaissezVib ? IconCode.LV_OUTSIDE : IconCode.TIE_OUTSIDE, value: SlurTieTypes.TIE_PLACEMENT_OUTSIDE, title: qsTrc("inspector", "Outside")  }
        ]
    }

    SpinBoxPropertyView {
        id: minLengthSection

        titleText: qsTrc("inspector", "Minimum length")
        visible: root.model ? root.model.isMinLengthAvailable : false
        propertyItem: root.model ? root.model.minLength : null

        step: 0.1
        maxValue: 10.00
        minValue: 1
        decimals: 2

        navigationName: "Minimum length"
        navigationPanel: root.navigationPanel
        navigationRowStart: root.tiePlacementSection + 1
    }
}
