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
import "internal"

Column {
    id: root

    property QtObject model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "SlurAndTieSettings"

    spacing: 12

    function focusOnFirst() {
        styleSection.focusOnFirst()
    }

    LineStyleSection {
        id: styleSection

        lineStyle: root.model ? root.model.lineStyle : null
        possibleLineStyles: root.model ? root.model.possibleLineStyles() : []

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart + 1
    }

    SeparatorLine { anchors.margins: -12 }

    PlacementSection {
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
        visible: root.model ? root.model.isTiePlacementAvailable : false
        propertyItem: root.model ? root.model.tiePlacement : null
        titleText: qsTrc("inspector", "Tie placement")

        navigationPanel: root.navigationPanel
        navigationRowStart: styleSection.navigationRowEnd + 1

        model: [
            { text: qsTrc("inspector", "Auto"), value: SlurTieTypes.TIE_PLACEMENT_AUTO },
            { text: qsTrc("inspector", "Inside"), value: SlurTieTypes.TIE_PLACEMENT_INSIDE },
            { text: qsTrc("inspector", "Outside"), value: SlurTieTypes.TIE_PLACEMENT_OUTSIDE }
        ]
    }
}
