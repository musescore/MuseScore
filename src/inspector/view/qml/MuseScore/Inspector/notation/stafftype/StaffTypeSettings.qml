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
    property int navigationRowOffset: 1

    objectName: "StaffTypeSettings"

    spacing: 12

    CheckBox {
        isIndeterminate: root.model ? root.model.isSmall.isUndefined : false
        checked: root.model && !isIndeterminate ? root.model.isSmall.value : false
        text: qsTrc("inspector", "Cue size")

        navigation.name: "CuteSizeCheckBox"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRowOffset + 1
        navigation.enabled: root.enabled

        onClicked: { root.model.isSmall.value = !checked }
    }

    Item {
        height: childrenRect.height
        width: parent.width

        OffsetSection {
            id: sizeOffset
            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 2

            verticalOffset: root.model ? root.model.verticalOffset : null

            verticalOffsetControl.step: 0.1
            verticalOffsetControl.decimals: 2
            verticalOffsetControl.minValue: 0.1
            verticalOffsetControl.maxValue: 5

            navigation.panel: root.navigationPanel
            navigationRowStart: root.navigationRowOffset + 1
        }

        SpinBoxPropertyView {
            id: scaleSection
            anchors.left: parent.horizontalCenter
            anchors.leftMargin: 2
            anchors.right: parent.right

            titleText: qsTrc("inspector", "Scale")
            propertyItem: root.model ? root.model.scale : null

            measureUnitsSymbol: "%"
            step: 20
            decimals: 2
            maxValue: 400
            minValue: 20

            navigation.panel: root.navigationPanel
            navigationRowStart: sizeOffset.navigationRowEnd + 1
        }
    }

    SeparatorLine { anchors.margins: -10 }

    Item {
        height: childrenRect.height
        width: parent.width

        SpinBoxPropertyView {
            id: numberOfLinesSection
            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 2

            titleText: qsTrc("inspector", "Number of lines")
            propertyItem: root.model ? root.model.lineCount : null

            step: 1
            decimals: 0
            maxValue: 14
            minValue: 1

            navigation.panel: root.navigationPanel
            navigationRowStart: scaleSection.navigationRowEnd + 1
        }

        SpinBoxPropertyView {
            id: lineDistance
            anchors.left: parent.horizontalCenter
            anchors.leftMargin: 2
            anchors.right: parent.right

            titleText: qsTrc("inspector", "Line distance")
            propertyItem: root.model ? root.model.lineDistance : null

            step: 0.25
            maxValue: 3
            minValue: 0

            navigation.panel: root.navigationPanel
            navigationRowStart: numberOfLinesSection.navigationRowEnd + 1
        }
    }

    SpinBoxPropertyView {
        id: stepOffset
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 2

        titleText: qsTrc("inspector", "Step offset")
        propertyItem: root.model ? root.model.stepOffset : null

        step: 1
        decimals: 0
        maxValue: 8
        minValue: -8

        navigation.panel: root.navigationPanel
        navigationRowStart: lineDistance.navigationRowEnd + 1
    }

    CheckBox {
        id: invisibleStaffLinesCheckBox
        isIndeterminate: root.model ? root.model.isInvisible.isUndefined : false
        checked: root.model && !isIndeterminate ? root.model.isInvisible.value : false
        text: qsTrc("inspector", "Invisible staff lines")

        navigation.name: "InvisibleStaffLinesCheckBox"
        navigation.panel: root.navigationPanel
        navigation.row: stepOffset.navigationRowEnd + 1
        navigation.enabled: root.enabled

        onClicked: { root.model.isInvisible.value = !checked }
    }

    ColorSection {
        id: staffLineColorSection
        titleText: qsTrc("inspector", "Staff line color")
        propertyItem: root.model ? root.model.color : null

        navigation.panel: root.navigationPanel
        navigationRowStart: invisibleStaffLinesCheckBox.navigation.row + 1
    }

    SeparatorLine { anchors.margins: -10 }

    DropdownPropertyView {
        id: noteHeadScheme
        titleText: qsTrc("inspector", "Notehead scheme")
        propertyItem: root.model ? root.model.noteheadSchemeType : null

        navigation.panel: root.navigationPanel
        navigationRowStart: staffLineColorSection.navigationRowEnd + 1

        model: [
            { text: qsTrc("inspector", "Auto"), value: NoteHead.SCHEME_AUTO },
            { text: qsTrc("inspector", "Normal"), value: NoteHead.SCHEME_NORMAL },
            { text: qsTrc("inspector", "Pitch names"), value: NoteHead.SCHEME_PITCHNAME },
            { text: qsTrc("inspector", "German pitch names"), value: NoteHead.SCHEME_PITCHNAME_GERMAN },
            { text: qsTrc("inspector", "Solfege movable Do"), value: NoteHead.SCHEME_SOLFEGE },
            { text: qsTrc("inspector", "Solfege fixed Do"), value: NoteHead.SCHEME_SOLFEGE_FIXED },
            { text: qsTrc("inspector", "4-shape (Walker)"), value: NoteHead.SCHEME_SHAPE_NOTE_4 },
            { text: qsTrc("inspector", "7-shape (Aikin)"), value: NoteHead.SCHEME_SHAPE_NOTE_7_AIKIN },
            { text: qsTrc("inspector", "7-shape (Funk)"), value: NoteHead.SCHEME_SHAPE_NOTE_7_FUNK },
            { text: qsTrc("inspector", "7-shape (Walker)"), value: NoteHead.SCHEME_SHAPE_NOTE_7_WALKER }
        ]
    }

    Column {
        spacing: 6

        width: parent.width

        CheckBox {
            isIndeterminate: root.model ? root.model.isStemless.isUndefined : false
            checked: root.model && !isIndeterminate ? root.model.isStemless.value : false
            text: qsTrc("inspector", "Stemless")

            navigation.name: "StremlessCheckBox"
            navigation.panel: root.navigationPanel
            navigation.row: noteHeadScheme.navigationRowEnd + 1
            navigation.enabled: root.enabled

            onClicked: { root.model.isStemless.value = !checked }
        }

        CheckBox {
            isIndeterminate: root.model ? root.model.shouldShowBarlines.isUndefined : false
            checked: root.model && !isIndeterminate ? root.model.shouldShowBarlines.value : false
            text: qsTrc("inspector", "Show barlines")

            navigation.name: "ShowBarlinesCheckBox"
            navigation.panel: root.navigationPanel
            navigation.row: noteHeadScheme.navigationRowEnd + 2
            navigation.enabled: root.enabled

            onClicked: { root.model.shouldShowBarlines.value = !checked }
        }

        CheckBox {
            isIndeterminate: root.model ? root.model.shouldShowLedgerLines.isUndefined : false
            checked: root.model && !isIndeterminate ? root.model.shouldShowLedgerLines.value : false
            text: qsTrc("inspector", "Show ledger lines")

            navigation.name: "ShowLedgerLinesCheckBox"
            navigation.panel: root.navigationPanel
            navigation.row: noteHeadScheme.navigationRowEnd + 3
            navigation.enabled: root.enabled

            onClicked: { root.model.shouldShowLedgerLines.value = !checked }
        }

        CheckBox {
            isIndeterminate: root.model ? root.model.shouldGenerateClefs.isUndefined : false
            checked: root.model && !isIndeterminate ? root.model.shouldGenerateClefs.value : false
            text: qsTrc("inspector", "Generate clefs")

            navigation.name: "GenerateClefsCheckBox"
            navigation.panel: root.navigationPanel
            navigation.row: noteHeadScheme.navigationRowEnd + 4
            navigation.enabled: root.enabled

            onClicked: { root.model.shouldGenerateClefs.value = !checked }
        }

        CheckBox {
            isIndeterminate: root.model ? root.model.shouldGenerateTimeSignatures.isUndefined : false
            checked: root.model && !isIndeterminate ? root.model.shouldGenerateTimeSignatures.value : false
            text: qsTrc("inspector", "Generate time signatures")

            navigation.name: "GenerateTimeSignaturesCheckBox"
            navigation.panel: root.navigationPanel
            navigation.row: noteHeadScheme.navigationRowEnd + 5
            navigation.enabled: root.enabled

            onClicked: { root.model.shouldGenerateTimeSignatures.value = !checked }
        }

        CheckBox {
            isIndeterminate: root.model ? root.model.shouldGenerateKeySignatures.isUndefined : false
            checked: root.model && !isIndeterminate ? root.model.shouldGenerateKeySignatures.value : false
            text: qsTrc("inspector", "Generate key signatures")

            navigation.name: "GenerateKeySignaturesCheckBox"
            navigation.panel: root.navigationPanel
            navigation.row: noteHeadScheme.navigationRowEnd + 6
            navigation.enabled: root.enabled

            onClicked: { root.model.shouldGenerateKeySignatures.value = !checked }
        }
    }
}
