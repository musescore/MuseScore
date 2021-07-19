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

Column {
    id: root

    property QtObject model: null

    objectName: "ChordSymbolSettings"

    spacing: 16

    InspectorPropertyView {
        titleText: qsTrc("inspector", "Interpretation")
        propertyItem: root.model ? root.model.isLiteral : null

        RadioButtonGroup {
            id: interpretationTypeList

            height: 30
            width: parent.width

            model: [
                { textRole: qsTrc("inspector", "Literal"), valueRole: true },
                { textRole: qsTrc("inspector", "Jazz"), valueRole: false }
            ]

            delegate: FlatRadioButton {

                ButtonGroup.group: interpretationTypeList.radioButtonGroup

                checked: root.model && !root.model.isLiteral.isUndefined ? root.model.isLiteral.value === modelData["valueRole"]
                                                                         : false

                onToggled: {
                    root.model.isLiteral.value = modelData["valueRole"]
                }

                StyledTextLabel {
                    text: modelData["textRole"]
                }
            }
        }
    }

    InspectorPropertyView {
        titleText: qsTrc("inspector", "Voicing")
        propertyItem: root.model ? root.model.voicingType : null

        Dropdown {
            id: voicing

            width: parent.width

            model: [
                { text: qsTrc("inspector", "Auto"), value: ChordSymbolTypes.VOICING_AUTO },
                { text: qsTrc("inspector", "Root only"), value: ChordSymbolTypes.VOICING_ROOT_ONLY },
                { text: qsTrc("inspector", "Close"), value: ChordSymbolTypes.VOICING_CLOSE },
                { text: qsTrc("inspector", "Drop two"), value: ChordSymbolTypes.VOICING_DROP_TWO },
                { text: qsTrc("inspector", "Six note"), value: ChordSymbolTypes.VOICING_SIX_NOTE },
                { text: qsTrc("inspector", "Four note"), value: ChordSymbolTypes.VOICING_FOUR_NOTE },
                { text: qsTrc("inspector", "Three note"), value: ChordSymbolTypes.VOICING_THREE_NOTE }
            ]

            currentIndex: root.model && !root.model.voicingType.isUndefined ? voicing.indexOfValue(root.model.voicingType.value) : -1

            onCurrentValueChanged: {
                if (currentIndex === -1) {
                    return
                }

                root.model.voicingType.value = voicing.currentValue
            }
        }
    }

    InspectorPropertyView {
        titleText: qsTrc("inspector", "Duration")
        propertyItem: root.model ? root.model.durationType : null

        Dropdown {
            id: durations

            width: parent.width

            model: [
                { text: qsTrc("inspector", "Until the next chord symbol"), value: ChordSymbolTypes.DURATION_UNTIL_NEXT_CHORD_SYMBOL },
                { text: qsTrc("inspector", "Until the end of the bar"), value: ChordSymbolTypes.DURATION_STOP_AT_MEASURE_END },
                { text: qsTrc("inspector", "Until the end of the attached duration"), value: ChordSymbolTypes.DURATION_SEGMENT_DURATION }
            ]

            currentIndex: root.model && !root.model.durationType.isUndefined ? durations.indexOfValue(root.model.durationType.value) : -1

            onCurrentValueChanged: {
                if (currentIndex === -1) {
                    return
                }

                root.model.durationType.value = durations.currentValue
            }
        }
    }
}
