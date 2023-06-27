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

    objectName: "AccidentalSettings"

    spacing: 12

    function focusOnFirst() {
        bracketType.focusOnFirst()
    }

    FlatRadioButtonGroupPropertyView {
        id: bracketType
        titleText: qsTrc("inspector", "Bracket type")
        propertyItem: root.model ? root.model.bracketType : null

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart

        model: [
            { text: qsTrc("inspector", "None"), value: AccidentalTypes.BRACKET_TYPE_NONE },
            { iconCode: IconCode.BRACKET_PARENTHESES, value: AccidentalTypes.BRACKET_TYPE_PARENTHESIS, title: qsTrc("inspector", "Parentheses") },
            { iconCode: IconCode.BRACKET_PARENTHESES_SQUARE, value: AccidentalTypes.BRACKET_TYPE_SQUARE, title: qsTrc("inspector", "Brackets") }
        ]
    }

    PropertyCheckBox {
        id: smallAccidentalCheckBox

        text: qsTrc("inspector", "Small accidental")
        propertyItem: root.model ? root.model.isSmall : null

        navigation.name: "SmallAccidentalBox"
        navigation.panel: root.navigationPanel
        navigation.row: bracketType.navigation.row + 1
    }
}
