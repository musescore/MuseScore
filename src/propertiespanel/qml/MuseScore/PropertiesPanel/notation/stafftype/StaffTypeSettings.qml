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
import QtQuick.Layouts

import Muse.Ui
import Muse.UiComponents
import MuseScore.PropertiesPanel

import "../../common"

Column {
    id: root

    required property StaffTypeSettingsModel model

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "StaffTypeSettings"

    spacing: 12

    function focusOnFirst() {
        cueSize.navigation.requestActive()
    }

    TextSection {
        titleText: qsTrc("propertiespanel", "Staff label")
        propertyItem: root.model ? root.model.staffLongName : null
    }

    TextSection {
        titleText: qsTrc("propertiespanel", "Abbreviated staff label")
        propertyItem: root.model ? root.model.staffShortName : null
    }

    SeparatorLine { anchors.margins: -12 }

    PropertyCheckBox {
        id: cueSize
        text: qsTrc("propertiespanel", "Cue size")
        propertyItem: root.model ? root.model.isSmall : null

        navigation.name: "CueSizeCheckBox"
        navigation.panel: root.navigationPanel
        navigation.row: root.navigationRowStart + 1
    }

    Item {
        height: childrenRect.height
        width: parent.width

        SpinBoxPropertyView {
            id: sizeOffset
            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 2

            titleText: qsTrc("propertiespanel", "Offset")
            propertyItem: root.model ? root.model.verticalOffset : null

            measureUnitsSymbol: qsTrc("global", "sp")
            step: 0.25
            decimals: 2
            maxValue: 20
            minValue: -20

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart + 1
        }

        SpinBoxPropertyView {
            id: scaleSection
            anchors.left: parent.horizontalCenter
            anchors.leftMargin: 2
            anchors.right: parent.right

            titleText: qsTrc("propertiespanel", "Scale")
            propertyItem: root.model ? root.model.scale : null

            measureUnitsSymbol: "%"
            step: 20
            decimals: 2
            maxValue: 400
            minValue: 20

            navigationPanel: root.navigationPanel
            navigationRowStart: sizeOffset.navigationRowEnd + 1
        }
    }

    Item {
        height: childrenRect.height
        width: parent.width

        SpinBoxPropertyView {
            id: numberOfLinesSection
            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 2

            titleText: qsTrc("propertiespanel", "Number of lines")
            propertyItem: root.model ? root.model.lineCount : null

            step: 1
            decimals: 0
            maxValue: 14
            minValue: 1

            navigationPanel: root.navigationPanel
            navigationRowStart: scaleSection.navigationRowEnd + 1
        }

        SpinBoxPropertyView {
            id: lineDistance
            anchors.left: parent.horizontalCenter
            anchors.leftMargin: 2
            anchors.right: parent.right

            titleText: qsTrc("propertiespanel", "Line distance")
            propertyItem: root.model ? root.model.lineDistance : null

            step: 0.25
            decimals: 2
            maxValue: 3
            minValue: 0

            navigationPanel: root.navigationPanel
            navigationRowStart: numberOfLinesSection.navigationRowEnd + 1
        }
    }

    SpinBoxPropertyView {
        id: stepOffset
        anchors.left: parent.left
        anchors.right: parent.horizontalCenter
        anchors.rightMargin: 2

        titleText: qsTrc("propertiespanel", "Step offset")
        propertyItem: root.model ? root.model.stepOffset : null

        step: 1
        decimals: 0
        maxValue: 8
        minValue: -8

        navigationPanel: root.navigationPanel
        navigationRowStart: lineDistance.navigationRowEnd + 1
    }

    PropertyCheckBox {
        id: invisibleStaffLinesCheckBox
        text: qsTrc("propertiespanel", "Invisible staff lines")
        propertyItem: root.model ? root.model.isInvisible : null

        navigation.name: "InvisibleStaffLinesCheckBox"
        navigation.panel: root.navigationPanel
        navigation.row: stepOffset.navigationRowEnd + 1
    }

    ColorSection {
        id: staffLineColorSection
        titleText: qsTrc("propertiespanel", "Staff line color")
        propertyItem: root.model ? root.model.color : null

        navigationPanel: root.navigationPanel
        navigationRowStart: invisibleStaffLinesCheckBox.navigation.row + 1
    }

    SeparatorLine { anchors.margins: -12 }

    DropdownPropertyView {
        id: noteHeadScheme
        titleText: qsTrc("propertiespanel", "Notehead scheme")
        propertyItem: root.model ? root.model.noteheadSchemeType : null

        navigationPanel: root.navigationPanel
        navigationRowStart: staffLineColorSection.navigationRowEnd + 1

        model: [
            { text: qsTrc("propertiespanel", "Auto"), value: NoteHead.SCHEME_AUTO },
            { text: qsTrc("propertiespanel", "Normal"), value: NoteHead.SCHEME_NORMAL },
            { text: qsTrc("propertiespanel", "Pitch names"), value: NoteHead.SCHEME_PITCHNAME },
            { text: qsTrc("propertiespanel", "Pitch names, no accidentals"), value: NoteHead.SCHEME_PITCHNAME_NO_ACCIDENTALS },
            { text: qsTrc("propertiespanel", "German pitch names"), value: NoteHead.SCHEME_PITCHNAME_GERMAN },
            { text: qsTrc("propertiespanel", "German pitch names, no accidentals"), value: NoteHead.SCHEME_PITCHNAME_GERMAN_NO_ACCIDENTALS },
            { text: qsTrc("propertiespanel", "Solfege movable Do"), value: NoteHead.SCHEME_SOLFEGE },
            { text: qsTrc("propertiespanel", "Solfege fixed Do"), value: NoteHead.SCHEME_SOLFEGE_FIXED },
            { text: qsTrc("propertiespanel", "4-shape (Walker)"), value: NoteHead.SCHEME_SHAPE_NOTE_4 },
            { text: qsTrc("propertiespanel", "7-shape (Aikin)"), value: NoteHead.SCHEME_SHAPE_NOTE_7_AIKIN },
            { text: qsTrc("propertiespanel", "7-shape (Funk)"), value: NoteHead.SCHEME_SHAPE_NOTE_7_FUNK },
            { text: qsTrc("propertiespanel", "7-shape (Walker)"), value: NoteHead.SCHEME_SHAPE_NOTE_7_WALKER }
        ]
    }

    Column {
        spacing: 6

        width: parent.width

        PropertyCheckBox {
            text: qsTrc("propertiespanel", "Stemless")
            propertyItem: root.model ? root.model.isStemless : null

            navigation.name: "StremlessCheckBox"
            navigation.panel: root.navigationPanel
            navigation.row: noteHeadScheme.navigationRowEnd + 1
        }

        PropertyCheckBox {
            text: qsTrc("propertiespanel", "Show barlines")
            propertyItem: root.model ? root.model.shouldShowBarlines : null

            navigation.name: "ShowBarlinesCheckBox"
            navigation.panel: root.navigationPanel
            navigation.row: noteHeadScheme.navigationRowEnd + 2
        }

        PropertyCheckBox {
            text: qsTrc("propertiespanel", "Show ledger lines")
            propertyItem: root.model ? root.model.shouldShowLedgerLines : null

            navigation.name: "ShowLedgerLinesCheckBox"
            navigation.panel: root.navigationPanel
            navigation.row: noteHeadScheme.navigationRowEnd + 3
        }

        PropertyCheckBox {
            text: qsTrc("propertiespanel", "Generate clefs")
            propertyItem: root.model ? root.model.shouldGenerateClefs : null

            navigation.name: "GenerateClefsCheckBox"
            navigation.panel: root.navigationPanel
            navigation.row: noteHeadScheme.navigationRowEnd + 4
        }

        PropertyCheckBox {
            text: qsTrc("propertiespanel", "Generate time signatures")
            propertyItem: root.model ? root.model.shouldGenerateTimeSignatures : null

            navigation.name: "GenerateTimeSignaturesCheckBox"
            navigation.panel: root.navigationPanel
            navigation.row: noteHeadScheme.navigationRowEnd + 5
        }

        PropertyCheckBox {
            text: qsTrc("propertiespanel", "Generate key signatures")
            propertyItem: root.model ? root.model.shouldGenerateKeySignatures : null

            navigation.name: "GenerateKeySignaturesCheckBox"
            navigation.panel: root.navigationPanel
            navigation.row: noteHeadScheme.navigationRowEnd + 6
        }
    }
}
