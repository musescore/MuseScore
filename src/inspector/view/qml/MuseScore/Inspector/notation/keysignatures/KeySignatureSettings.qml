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

import MuseScore.UiComponents 1.0
import MuseScore.Inspector 1.0
import "../../common"

Column {
    id: root

    property QtObject model: null

    objectName: "KeySignatureSettings"

    spacing: 16

    CheckBox {
        isIndeterminate: root.model ? root.model.hasToShowCourtesy.isUndefined : false
        checked: root.model && !isIndeterminate ? root.model.hasToShowCourtesy.value : false
        text: qsTrc("inspector", "Show courtesy key signature on previous system")

        onClicked: { root.model.hasToShowCourtesy.value = !checked }
    }

    InspectorPropertyView {

        titleText: qsTrc("inspector", "Mode")
        propertyItem: root.model ? root.model.mode : null

        Dropdown {
            id: modes

            width: parent.width

            model: [
                { text: qsTrc("inspector", "Unknown"), value: KeySignatureTypes.MODE_UNKNOWN },
                { text: qsTrc("inspector", "None"), value: KeySignatureTypes.MODE_NONE },
                { text: qsTrc("inspector", "Major"), value: KeySignatureTypes.MODE_MAJOR },
                { text: qsTrc("inspector", "Minor"), value: KeySignatureTypes.MODE_MINOR },
                { text: qsTrc("inspector", "Dorian"), value: KeySignatureTypes.MODE_DORIAN },
                { text: qsTrc("inspector", "Phrygian"), value: KeySignatureTypes.MODE_PHRYGIAN },
                { text: qsTrc("inspector", "Lydian"), value: KeySignatureTypes.MODE_LYDIAN },
                { text: qsTrc("inspector", "Mixolydian"), value: KeySignatureTypes.MODE_MIXOLYDIAN },
                { text: qsTrc("inspector", "Ionian"), value: KeySignatureTypes.MODE_IONIAN },
                { text: qsTrc("inspector", "Locrian"), value: KeySignatureTypes.MODE_LOCRIAN }
            ]

            currentIndex: root.model && !root.model.mode.isUndefined ? modes.indexOfValue(root.model.mode.value) : -1

            onCurrentValueChanged: {
                if (currentIndex === -1) {
                    return
                }

                root.model.mode.value = modes.currentValue
            }
        }
    }
}
