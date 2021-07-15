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

import MuseScore.UiComponents 1.0
import MuseScore.Inspector 1.0
import "../../common"

InspectorPropertyView {
    id: root

    property QtObject model: null

    objectName: "AccidentalSettings"

    titleText: qsTrc("inspector", "Bracket type")
    propertyItem: root.model ? root.model.bracketType : null

    RadioButtonGroup {
        id: radioButtonList

        height: 30
        width: parent.width

        model: [
            { textRole: qsTrc("inspector", "None"), valueRole: AccidentalTypes.BRACKET_TYPE_NONE },
            { textRole: qsTrc("inspector", "Parenthesis"), valueRole: AccidentalTypes.BRACKET_TYPE_PARENTHESIS },
            { textRole: qsTrc("inspector", "Bracket"), valueRole: AccidentalTypes.BRACKET_TYPE_SQUARE },
            { textRole: qsTrc("inspector", "Brace"), valueRole: AccidentalTypes.BRACKET_TYPE_ROUND },
        ]

        delegate: FlatRadioButton {
            id: radioButtonDelegate

            ButtonGroup.group: radioButtonList.radioButtonGroup

            checked: root.model && !root.model.bracketType.isUndefined ? root.model.bracketType.value === modelData["valueRole"]
                                                                       : false
            onToggled: {
                root.model.bracketType.value = modelData["valueRole"]
            }

            StyledTextLabel {
                text: modelData["textRole"]

                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }
}
