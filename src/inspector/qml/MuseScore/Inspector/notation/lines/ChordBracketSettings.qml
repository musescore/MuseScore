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
import QtQuick

import Muse.Ui
import Muse.UiComponents
import MuseScore.Inspector

import "../../common"
import "internal"

Column {
    id: root

    required property ChordBracketSettingsModel model

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1
    readonly property int navigationRowEnd: playCountText.navigationRowEnd

    spacing: 12

    FlatRadioButtonGroupPropertyView {
        id: leftRightSection

        visible: root.model && root.model.isBracket
        propertyItem: root.model ? root.model.bracketRightSide : null
        titleText: qsTrc("inspector", "Placement")

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart + 1

        model: [
            { text: qsTrc("inspector", "Left"), value: false},
            { text: qsTrc("inspector", "Right"), value: true},
        ]
    }

    FlatRadioButtonGroupPropertyView {
        id: hookPosSection

        visible: root.model && root.model.isBracket
        propertyItem: root.model ? root.model.hookPos : null
        titleText: qsTrc("inspector", "Hooks")

        navigationPanel: root.navigationPanel
        navigationRowStart: leftRightSection.navigationRowEnd + 1

        model: [
            { text: qsTrc("inspector", "Both"), value: 0},
            { text: qsTrc("inspector", "Above"), value: 1},
            { text: qsTrc("inspector", "Below"), value: 2},
        ]
    }

    SpinBoxPropertyView {
        id: hookLenSection

        titleText: qsTrc("inspector", "Hook length")
        visible: root.model && root.model.isBracket
        propertyItem: root.model ? root.model.hookLen : null

        step: 0.1
        maxValue: 10.00
        minValue: 0.01
        decimals: 2
        measureUnitsSymbol: "sp"

        navigationName: "Hook length"
        navigationPanel: root.navigationPanel
        navigationRowStart: hookPosSection.navigationRowEnd + 1
    }
}
