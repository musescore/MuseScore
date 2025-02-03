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

Column {
    id: root

    property QtObject model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "KeySignatureSettings"

    spacing: 12

    function focusOnFirst() {
        showCourtesyKeySignature.navigation.requestActive()
    }

    PropertyCheckBox {
        id: showCourtesyKeySignature
        text: qsTrc("inspector", "Show courtesy key signature")
        propertyItem: root.model ? root.model.hasToShowCourtesy : null

        navigation.name: "ShowCourtesyKeySignature"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRowStart + 1
    }

    DropdownPropertyView {
        //: Musical mode (major, minor, dorian, phrygian, lydian, etc.)
        titleText: qsTrc("inspector", "Mode")
        propertyItem: root.model ? root.model.mode : null

        navigationName: "Mode"
        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart + 2

        model: [
            { text: qsTrc("inspector", "Unknown"), value: KeySignatureTypes.MODE_UNKNOWN },
            { text: qsTrc("inspector", "None"), value: KeySignatureTypes.MODE_NONE },
            //: mode of a key signature, not an interval
            { text: qsTrc("inspector", "Major", "key signature mode"), value: KeySignatureTypes.MODE_MAJOR },
            //: mode of a key signature, not an interval
            { text: qsTrc("inspector", "Minor", "key signature mode"), value: KeySignatureTypes.MODE_MINOR },
            { text: qsTrc("inspector", "Dorian"), value: KeySignatureTypes.MODE_DORIAN },
            { text: qsTrc("inspector", "Phrygian"), value: KeySignatureTypes.MODE_PHRYGIAN },
            { text: qsTrc("inspector", "Lydian"), value: KeySignatureTypes.MODE_LYDIAN },
            { text: qsTrc("inspector", "Mixolydian"), value: KeySignatureTypes.MODE_MIXOLYDIAN },
            { text: qsTrc("inspector", "Aeolian"), value: KeySignatureTypes.MODE_AEOLIAN },
            { text: qsTrc("inspector", "Ionian"), value: KeySignatureTypes.MODE_IONIAN },
            { text: qsTrc("inspector", "Locrian"), value: KeySignatureTypes.MODE_LOCRIAN }
        ]
    }
}
