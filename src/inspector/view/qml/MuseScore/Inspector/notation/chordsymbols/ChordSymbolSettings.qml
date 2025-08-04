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
        id: durationSection
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

    PropertyCheckBox {
        id: verticalAlignCheckBox

        text: qsTrc("inspector", "Exclude from vertical alignment")
        propertyItem: root.model ? root.model.verticalAlign : null

        navigation.name: "Exclude from vertical alignment"
        navigation.panel: root.navigationPanel
        navigation.row: durationSection.navigationRowEnd + 1
    }

    PropertyCheckBox {
        id: doNotStackModifiersCheckBox

        text: qsTrc("inspector", "Do not stack modifiers")
        propertyItem: root.model ? root.model.doNotStackModifiers : null

        navigation.name: "Do not stack modifiers"
        navigation.panel: root.navigationPanel
        navigation.row: verticalAlignCheckBox.navigationRowEnd + 1
    }

    Item {
        height: childrenRect.height
        width: parent.width

        SpinBoxPropertyView {
            id: bassNoteScale

            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 4

            titleText: qsTrc("inspector", "Bass note scale")
            propertyItem: root.model ? root.model.bassScale : null

            step: 1
            decimals: 0
            maxValue: 300
            minValue: 0
            measureUnitsSymbol: "%"

            navigationName: "Bass note scale"
            navigationPanel: root.navigationPanel
            navigationRowStart: doNotStackModifiersCheckBox.navigationRowEnd + 1
        }

        FlatRadioButtonGroupPropertyView {
            id: alignmentButtonList

            anchors.left: parent.horizontalCenter
            anchors.leftMargin: 4
            anchors.right: parent.right

            transparent: true

            titleText: qsTrc("inspector", "Alignment to notehead")
            propertyItem: root.model ? root.model.position : null

            navigationPanel: root.navigationPanel
            navigationRowStart: bassNoteScale.navigationRowEnd + 1

            requestIconFontSize: 16
            requestWidth: 98

            model: [
                { iconCode: IconCode.NOTE_ALIGN_LEFT, value: 0},
                { iconCode: IconCode.NOTE_ALIGN_CENTER, value: 2},
                { iconCode: IconCode.NOTE_ALIGN_RIGHT, value: 1 }
            ]
        }
    }

    FlatButton {
        id: addFretBoardDiagramButton
        width: parent.width

        navigation.name: "AddFretboardDiagram"
        navigation.panel: root.navigationPanel
        navigation.row: alignmentButtonList.navigationRowEnd + 1

        text: qsTrc("inspector", "Add fretboard diagram")
        icon: IconCode.FRETBOARD_DIAGRAM
        orientation: Qt.Horizontal

        visible: root.model ? !root.model.hasLinkedFretboardDiagram : false

        onClicked: {
            if (root.model) {
                root.model.addFretboardDiagram()
            }
        }
    }
}
