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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../../common"

Column {
    id: root

    property QtObject model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "ArticulationSettings"

    spacing: 12

    function focusOnFirst() {
        directionSection.focusOnFirst()
    }

    FlatRadioButtonGroupPropertyView {
        id: directionSection
        titleText: qsTrc("inspector", "Direction")
        propertyItem: root.model ? root.model.direction : null

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart + 1

        model: [
            { text: qsTrc("inspector", "Auto"), value: ArticulationTypes.AUTO, title: qsTrc("inspector", "Auto") },
            { iconCode: IconCode.ARROW_DOWN, value: ArticulationTypes.DOWN, title: qsTrc("inspector", "Down") },
            { iconCode: IconCode.ARROW_UP, value: ArticulationTypes.UP, title: qsTrc("inspector", "Up") }
        ]
    }

    DropdownPropertyView {
        titleText: qsTrc("inspector", "Placement")
        propertyItem: root.model ? root.model.placement : null

        navigationPanel: root.navigationPanel
        navigationRowStart: directionSection.navigationRowEnd + 1

        model: [
            { text: qsTrc("inspector", "Above staff"), value: ArticulationTypes.TYPE_ABOVE_STAFF },
            { text: qsTrc("inspector", "Below staff"), value: ArticulationTypes.TYPE_BELOW_STAFF },
            { text: qsTrc("inspector", "Chord automatic"), value: ArticulationTypes.TYPE_CHORD_AUTO },
            { text: qsTrc("inspector", "Above chord"), value: ArticulationTypes.TYPE_ABOVE_CHORD },
            { text: qsTrc("inspector", "Below chord"), value: ArticulationTypes.TYPE_BELOW_CHORD }
        ]
    }
}
