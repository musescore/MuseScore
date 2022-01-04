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

    objectName: "TupletSettings"

    spacing: 12

    function focusOnFirst() {
        directionSection.focusOnFirst()
    }

    FlatRadioButtonGroupPropertyView {
        id: directionSection
        titleText: qsTrc("inspector", "Direction")
        propertyItem: root.model ? root.model.directionType : null

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart + 1

        model: root.model ? root.model.possibleDirectionTypes() : null
    }

    FlatRadioButtonGroupPropertyView {
        id: numberTypeSection
        titleText: qsTrc("inspector", "Number type")
        propertyItem: root.model ? root.model.numberType : null

        navigationPanel: root.navigationPanel
        navigationRowStart: directionSection.navigationRowEnd + 1

        model: root.model ? root.model.possibleNumberTypes() : null
    }

    FlatRadioButtonGroupPropertyView {
        id: bracketTypeSection
        titleText: qsTrc("inspector", "Bracket type")
        propertyItem: root.model ? root.model.bracketType : null

        navigationPanel: root.navigationPanel
        navigationRowStart: numberTypeSection.navigationRowEnd + 1

        model: root.model ? root.model.possibleBracketTypes() : null
    }

    SeparatorLine { anchors.margins: -12 }

    SpinBoxPropertyView {
        id: lineThicknessSection
        titleText: qsTrc("inspector", "Line thickness")
        propertyItem: root.model ? root.model.lineThickness : null

        navigationPanel: root.navigationPanel
        navigationRowStart: bracketTypeSection.navigationRowEnd + 1

        minValue: 0.1
        maxValue: 1
        step: 0.1
    }
}
