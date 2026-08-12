/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
import MuseScore.PropertiesPanel

import "../../common"

Column {
    id: root

    required property KeySignatureSettingsModel model

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "KeySignatureSettings"

    spacing: 12

    function focusOnFirst() {
        showCourtesyKeySignature.navigation.requestActive()
    }

    PropertyCheckBox {
        id: showCourtesyKeySignature
        text: qsTrc("propertiespanel", "Show courtesy key signature")
        propertyItem: root.model ? root.model.hasToShowCourtesy : null

        navigation.name: "ShowCourtesyKeySignature"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRowStart + 1
    }

    DropdownPropertyView {
        //: Musical mode (major, minor, dorian, phrygian, lydian, etc.)
        titleText: qsTrc("propertiespanel", "Mode")
        propertyItem: root.model ? root.model.mode : null

        navigationName: "Mode"
        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart + 2

        model: [
            { text: qsTrc("propertiespanel", "Unknown", "key signature mode"), value: KeySignatureTypes.MODE_UNKNOWN },
            { text: qsTrc("propertiespanel", "None", "key signature mode"), value: KeySignatureTypes.MODE_NONE },
            //: mode of a key signature, not an interval
            { text: qsTrc("propertiespanel", "Major", "key signature mode"), value: KeySignatureTypes.MODE_MAJOR },
            //: mode of a key signature, not an interval
            { text: qsTrc("propertiespanel", "Minor", "key signature mode"), value: KeySignatureTypes.MODE_MINOR },
            { text: qsTrc("propertiespanel", "Dorian", "key signature mode"), value: KeySignatureTypes.MODE_DORIAN },
            { text: qsTrc("propertiespanel", "Phrygian", "key signature mode"), value: KeySignatureTypes.MODE_PHRYGIAN },
            { text: qsTrc("propertiespanel", "Lydian", "key signature mode"), value: KeySignatureTypes.MODE_LYDIAN },
            { text: qsTrc("propertiespanel", "Mixolydian", "key signature mode"), value: KeySignatureTypes.MODE_MIXOLYDIAN },
            { text: qsTrc("propertiespanel", "Aeolian", "key signature mode"), value: KeySignatureTypes.MODE_AEOLIAN },
            { text: qsTrc("propertiespanel", "Ionian", "key signature mode"), value: KeySignatureTypes.MODE_IONIAN },
            { text: qsTrc("propertiespanel", "Locrian", "key signature mode"), value: KeySignatureTypes.MODE_LOCRIAN }
        ]
    }
}
