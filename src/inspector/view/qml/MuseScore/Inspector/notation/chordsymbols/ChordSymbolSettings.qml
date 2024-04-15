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
import QtQuick.Controls 2.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../../common"

Column {
    id: root

    property QtObject model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "ChordSymbolSettings"

    spacing: 12

    function focusOnFirst() {
        interpretationSection.focusOnFirst()
    }

    FlatRadioButtonGroupPropertyView {
        id: interpretationSection
        titleText: qsTrc("inspector", "Interpretation")
        propertyItem: root.model ? root.model.isLiteral : null

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart + 1

        model: [
            { text: qsTrc("inspector", "Literal"), value: true },
            { text: qsTrc("inspector", "Jazz"), value: false }
        ]
    }

    DropdownPropertyView {
        id: voicingSection
        titleText: qsTrc("inspector", "Voicing")
        propertyItem: root.model ? root.model.voicingType : null

        navigationPanel: root.navigationPanel
        navigationRowStart: interpretationSection.navigationRowEnd + 1

        model: [
            { text: qsTrc("inspector", "Auto"), value: ChordSymbolTypes.VOICING_AUTO },
            { text: qsTrc("inspector", "Root only"), value: ChordSymbolTypes.VOICING_ROOT_ONLY },
            { text: qsTrc("inspector", "Close"), value: ChordSymbolTypes.VOICING_CLOSE },
            { text: qsTrc("inspector", "Drop two"), value: ChordSymbolTypes.VOICING_DROP_TWO },
            { text: qsTrc("inspector", "Six note"), value: ChordSymbolTypes.VOICING_SIX_NOTE },
            { text: qsTrc("inspector", "Four note"), value: ChordSymbolTypes.VOICING_FOUR_NOTE },
            { text: qsTrc("inspector", "Three note"), value: ChordSymbolTypes.VOICING_THREE_NOTE }
        ]
    }

    DropdownPropertyView {
        titleText: qsTrc("inspector", "Duration")
        propertyItem: root.model ? root.model.durationType : null

        navigationPanel: root.navigationPanel
        navigationRowStart: voicingSection.navigationRowEnd + 1

        model: [
            { text: qsTrc("inspector", "Until the next chord symbol"), value: ChordSymbolTypes.DURATION_UNTIL_NEXT_CHORD_SYMBOL },
            { text: qsTrc("inspector", "Until the end of the measure"), value: ChordSymbolTypes.DURATION_STOP_AT_MEASURE_END },
            { text: qsTrc("inspector", "Until the end of the attached duration"), value: ChordSymbolTypes.DURATION_SEGMENT_DURATION }
        ]
    }
}
