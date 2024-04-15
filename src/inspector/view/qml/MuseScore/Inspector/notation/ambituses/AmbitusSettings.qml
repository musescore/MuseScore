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
import "../notes/internal"

Column {
    id: root

    property QtObject model: null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    objectName: "AmbitusSettings"

    spacing: 12

    readonly property var tpcListModel: [
        { text: qsTrc("global", "C♭♭"), value: AmbitusTypes.TPC_C_BB },
        { text: qsTrc("global", "C♭"), value: AmbitusTypes.TPC_C_B },
        { text: qsTrc("global", "C"), value: AmbitusTypes.TPC_C },
        { text: qsTrc("global", "C♯"), value: AmbitusTypes.TPC_C_S },
        { text: qsTrc("global", "C♯♯"), value: AmbitusTypes.TPC_C_SS },
        { text: qsTrc("global", "D♭♭"), value: AmbitusTypes.TPC_D_BB },
        { text: qsTrc("global", "D♭"), value: AmbitusTypes.TPC_D_B },
        { text: qsTrc("global", "D"), value: AmbitusTypes.TPC_D },
        { text: qsTrc("global", "D♯"), value: AmbitusTypes.TPC_D_S },
        { text: qsTrc("global", "D♯♯"), value: AmbitusTypes.TPC_D_SS },
        { text: qsTrc("global", "E♭♭"), value: AmbitusTypes.TPC_E_BB },
        { text: qsTrc("global", "E♭"), value: AmbitusTypes.TPC_E_B },
        { text: qsTrc("global", "E"), value: AmbitusTypes.TPC_E },
        { text: qsTrc("global", "E♯"), value: AmbitusTypes.TPC_E_S },
        { text: qsTrc("global", "E♯♯"), value: AmbitusTypes.TPC_E_SS },
        { text: qsTrc("global", "F♭♭"), value: AmbitusTypes.TPC_F_BB },
        { text: qsTrc("global", "F♭"), value: AmbitusTypes.TPC_F_B },
        { text: qsTrc("global", "F"), value: AmbitusTypes.TPC_F },
        { text: qsTrc("global", "F♯"), value: AmbitusTypes.TPC_F_S },
        { text: qsTrc("global", "F♯♯"), value: AmbitusTypes.TPC_F_SS },
        { text: qsTrc("global", "G♭♭"), value: AmbitusTypes.TPC_G_BB },
        { text: qsTrc("global", "G♭"), value: AmbitusTypes.TPC_G_B },
        { text: qsTrc("global", "G"), value: AmbitusTypes.TPC_G },
        { text: qsTrc("global", "G♯"), value: AmbitusTypes.TPC_G_S },
        { text: qsTrc("global", "G♯♯"), value: AmbitusTypes.TPC_G_SS },
        { text: qsTrc("global", "A♭♭"), value: AmbitusTypes.TPC_A_BB },
        { text: qsTrc("global", "A♭"), value: AmbitusTypes.TPC_A_B },
        { text: qsTrc("global", "A"), value: AmbitusTypes.TPC_A },
        { text: qsTrc("global", "A♯"), value: AmbitusTypes.TPC_A_S },
        { text: qsTrc("global", "A♯♯"), value: AmbitusTypes.TPC_A_SS },
        { text: qsTrc("global", "B♭♭"), value: AmbitusTypes.TPC_B_BB },
        { text: qsTrc("global", "B♭"), value: AmbitusTypes.TPC_B_B },
        { text: qsTrc("global", "B"), value: AmbitusTypes.TPC_B },
        { text: qsTrc("global", "B♯"), value: AmbitusTypes.TPC_B_S },
        { text: qsTrc("global", "B♯♯"), value: AmbitusTypes.TPC_B_SS }
    ]

    height: childrenRect.height

    function focusOnFirst() {
        topNoteSection.focusOnFirst()
    }

    Item {
        height: childrenRect.height
        width: parent.width

        DropdownPropertyView {
            id: topNoteSection
            propertyItem: root.model ? root.model.topTpc : null
            titleText: qsTrc("inspector", "Top note")
            showButton: false

            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 2

            model: root.tpcListModel

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart + 1
        }

        SpinBoxPropertyView {
            id: topOctaveSection
            propertyItem: root.model ? root.model.topOctave : null
            showTitle: true // Show empty label for correct alignment
            showButton: false

            anchors.left: parent.horizontalCenter
            anchors.leftMargin: 2
            anchors.right: parent.right

            step: 1
            decimals: 0
            maxValue: 8
            minValue: -1

            navigationPanel: root.navigationPanel
            navigationRowStart: topNoteSection.navigationRowEnd + 1
        }
    }

    Item {
        height: childrenRect.height
        width: parent.width

        DropdownPropertyView {
            id: bottomNoteSection
            propertyItem: root.model ? root.model.bottomTpc : null
            titleText: qsTrc("inspector", "Bottom note")
            showButton: false

            anchors.left: parent.left
            anchors.right: parent.horizontalCenter
            anchors.rightMargin: 2

            model: root.tpcListModel

            navigationPanel: root.navigationPanel
            navigationRowStart: topOctaveSection.navigationRowEnd + 1
        }

        SpinBoxPropertyView {
            id: bottomOctaveSection
            propertyItem: root.model ? root.model.bottomOctave : null
            showTitle: true // Show empty label for correct alignment
            showButton: false

            anchors.left: parent.horizontalCenter
            anchors.leftMargin: 2
            anchors.right: parent.right

            step: 1
            decimals: 0
            maxValue: 8
            minValue: -1

            navigationPanel: root.navigationPanel
            navigationRowStart: bottomNoteSection.navigationRowEnd + 1
        }
    }

    FlatButton {
        id: updateButton
        width: parent.width
        text: qsTrc("inspector", "Update to match the notes on the staff")

        navigation.name: "UpdateButton"
        navigation.panel: root.navigationPanel
        navigation.row: bottomOctaveSection.navigationRowEnd + 1

        onClicked: {
            if (root.model) {
                root.model.matchRangesToStaff()
            }
        }
    }

    ExpandableBlank {
        id: showItem
        isExpanded: false

        title: isExpanded ? qsTrc("inspector", "Show less") : qsTrc("inspector", "Show more")

        width: parent.width

        navigation.panel: root.navigationPanel
        navigation.row: updateButton.navigation.row + 1

        contentItemComponent: Column {
            height: implicitHeight
            width: parent.width

            spacing: 12

            FlatRadioButtonGroupPropertyView {
                id: directionSection
                titleText: qsTrc("inspector", "Direction")
                propertyItem: root.model ? root.model.direction : null

                navigationPanel: root.navigationPanel
                navigationRowStart: showItem.navigation.row + 1

                model: [
                    { iconCode: IconCode.AMBITUS, value: DirectionTypes.HORIZONTAL_AUTO, title: qsTrc("inspector", "Auto") },
                    { iconCode: IconCode.AMBITUS_LEANING_LEFT, value: DirectionTypes.HORIZONTAL_LEFT, title: qsTrc("inspector", "Left") },
                    { iconCode: IconCode.AMBITUS_LEANING_RIGHT, value: DirectionTypes.HORIZONTAL_RIGHT, title: qsTrc("inspector", "Right") }
                ]
            }

            NoteheadGroupSelector {
                id: noteheadGroup
                propertyItem: root.model ? root.model.noteheadGroup : null

                navigationPanel: root.navigationPanel
                navigationRowStart: directionSection.navigationRowEnd + 1
            }

            NoteheadTypeSelector {
                id: noteheadType
                propertyItem: root.model ? root.model.noteheadType : null

                navigationPanel: root.navigationPanel
                navigationRowStart: noteheadGroup.navigationRowEnd + 1
            }

            SpinBoxPropertyView {
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                titleText: qsTrc("inspector", "Line thickness")
                propertyItem: root.model ? root.model.lineThickness : null

                step: 0.1
                maxValue: 10
                minValue: 0.1
                decimals: 2

                navigationPanel: root.navigationPanel
                navigationRowStart: noteheadType.navigationRowEnd + 1
            }
        }
    }
}
