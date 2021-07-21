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
import "../notes/internal"

Column {
    id: root

    property QtObject model: null

    objectName: "AmbitusSettings"

    spacing: 12

    function tpcListModel() {
        return [
                    { text: "C♭♭", value: AmbitusTypes.TPC_C_BB },
                    { text: "C♭", value: AmbitusTypes.TPC_C_B },
                    { text: "C", value: AmbitusTypes.TPC_C },
                    { text: "C♯", value: AmbitusTypes.TPC_C_S },
                    { text: "C♯♯", value: AmbitusTypes.TPC_C_SS },
                    { text: "D♭♭", value: AmbitusTypes.TPC_D_BB },
                    { text: "D♭", value: AmbitusTypes.TPC_D_B },
                    { text: "D", value: AmbitusTypes.TPC_D },
                    { text: "D♯", value: AmbitusTypes.TPC_D_S },
                    { text: "D♯♯", value: AmbitusTypes.TPC_D_SS },
                    { text: "E♭♭", value: AmbitusTypes.TPC_E_BB },
                    { text: "E♭", value: AmbitusTypes.TPC_E_B },
                    { text: "E", value: AmbitusTypes.TPC_E },
                    { text: "E♯", value: AmbitusTypes.TPC_E_S },
                    { text: "E♯♯", value: AmbitusTypes.TPC_E_SS },
                    { text: "F♭♭", value: AmbitusTypes.TPC_F_BB },
                    { text: "F♭", value: AmbitusTypes.TPC_F_B },
                    { text: "F", value: AmbitusTypes.TPC_F },
                    { text: "F♯", value: AmbitusTypes.TPC_F_S },
                    { text: "F♯♯", value: AmbitusTypes.TPC_F_SS },
                    { text: "G♭♭", value: AmbitusTypes.TPC_G_BB },
                    { text: "G♭", value: AmbitusTypes.TPC_G_B },
                    { text: "G", value: AmbitusTypes.TPC_G },
                    { text: "G♯", value: AmbitusTypes.TPC_G_S },
                    { text: "G♯♯", value: AmbitusTypes.TPC_G_SS },
                    { text: "A♭♭", value: AmbitusTypes.TPC_A_BB },
                    { text: "A♭", value: AmbitusTypes.TPC_A_B },
                    { text: "A", value: AmbitusTypes.TPC_A },
                    { text: "A♯", value: AmbitusTypes.TPC_A_S },
                    { text: "A♯♯", value: AmbitusTypes.TPC_A_SS },
                    { text: "B♭♭", value: AmbitusTypes.TPC_B_BB },
                    { text: "B♭", value: AmbitusTypes.TPC_B_B },
                    { text: "B", value: AmbitusTypes.TPC_B },
                    { text: "B♯", value: AmbitusTypes.TPC_B_S },
                    { text: "B♯♯", value: AmbitusTypes.TPC_B_SS }
                ]
    }

    FlatButton {
        width: parent.width

        text: qsTrc("inspector", "Update to match the notes on the staff")

        onClicked: {
            if (root.model) {
                root.model.matchRangesToStaff()
            }
        }
    }

    NoteheadsGrid {
        id: noteheadGridView

        noteHeadTypesModel: root.model ? root.model.noteheadGroupsModel : null
    }

    Column {
        spacing: 8

        height: childrenRect.height
        width: parent.width

        Item {
            height: childrenRect.height
            width: parent.width

            InspectorPropertyView {
                titleText: qsTrc("inspector", "Top TPC")
                propertyItem: root.model ? root.model.topTpc : null

                Dropdown {
                    id: ttpcs
                    anchors.left: parent.left
                    anchors.right: parent.horizontalCenter
                    anchors.rightMargin: 2
                    width: parent.width

                    model: tpcListModel()

                    currentIndex: root.model && !root.model.topTpc.isUndefined ? ttpcs.indexOfValue(root.model.topTpc.value) : -1

                    onCurrentValueChanged: {
                        if (currentIndex === -1) {
                            return
                        }

                        root.model.topTpc.value = ttpcs.currentValue
                    }
                }
            }

            InspectorPropertyView {
                titleText: qsTrc("inspector", "Top octave")
                propertyItem: root.model ? root.model.topOctave : null

                IncrementalPropertyControl {
                    id: topOctaveControl

                    anchors.left: parent.horizontalCenter
                    anchors.leftMargin: 2
                    anchors.right: parent.right

                    iconMode: iconModeEnum.hidden
                    isIndeterminate: root.model ? root.model.topOctave.isUndefined : false
                    currentValue: root.model ? root.model.topOctave.value : 0

                    step: 1
                    decimals: 0
                    maxValue: 8
                    minValue: -1
                    validator: IntInputValidator {
                        top: topOctaveControl.maxValue
                        bottom: topOctaveControl.minValue
                    }

                    onValueEdited: { root.model.topOctave.value = newValue }
                }
            }
        }

        Item {
            height: childrenRect.height
            width: parent.width

            Dropdown {
                id: tpcs
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2
                width: parent.width

                model: tpcListModel()

                currentIndex: root.model && !root.model.bottomTpc.isUndefined ? tpcs.indexOfValue(root.model.bottomTpc.value) : -1

                onCurrentValueChanged: {
                    if (currentIndex === -1) {
                        return
                    }

                    root.model.bottomTpc.value = tpcs.currentValue
                }
            }

            IncrementalPropertyControl {
                id: bottomOctaveControl

                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                iconMode: iconModeEnum.hidden
                isIndeterminate: root.model ? root.model.bottomOctave.isUndefined : false
                currentValue: root.model ? root.model.bottomOctave.value : 0

                step: 1
                decimals: 0
                maxValue: 8
                minValue: -1
                validator: IntInputValidator {
                    top: bottomOctaveControl.maxValue
                    bottom: bottomOctaveControl.minValue
                }

                onValueEdited: { root.model.bottomOctave.value = newValue }
            }
        }

        ExpandableBlank {
            isExpanded: false

            title: isExpanded ? qsTrc("inspector", "Show less") : qsTrc("inspector", "Show more")

            width: parent.width

            contentItemComponent: Column {
                height: implicitHeight
                width: parent.width

                spacing: 16

                InspectorPropertyView {
                    titleText: qsTrc("inspector", "Direction")
                    propertyItem: root.model ? root.model.direction : null

                    RadioButtonGroup {
                        id: directionButtonList

                        height: 30
                        width: parent.width

                        model: [
                            { iconRole: IconCode.AMBITUS, typeRole: DirectionTypes.HORIZONTAL_AUTO },
                            { iconRole: IconCode.AMBITUS_LEANING_LEFT, typeRole: DirectionTypes.HORIZONTAL_LEFT },
                            { iconRole: IconCode.AMBITUS_LEANING_RIGHT, typeRole: DirectionTypes.HORIZONTAL_RIGHT }
                        ]

                        delegate: FlatRadioButton {

                            ButtonGroup.group: directionButtonList.radioButtonGroup

                            checked: root.model && !root.model.direction.isUndefined ? root.model.direction.value === modelData["typeRole"]
                                                                                     : false

                            onToggled: {
                                root.model.direction.value = modelData["typeRole"]
                            }

                            StyledIconLabel {
                                iconCode: modelData["iconRole"]
                            }
                        }
                    }
                }

                InspectorPropertyView {
                    titleText: qsTrc("inspector", "Head type (visual only)")
                    propertyItem: root.model ? root.model.noteheadType : null

                    RadioButtonGroup {
                        id: headTypeButtonList

                        height: 30
                        width: parent.width

                        model: [
                            { iconRole: IconCode.AUTO, typeRole: NoteHead.TYPE_AUTO },
                            { iconRole: IconCode.NOTE_HEAD_QUARTER, typeRole: NoteHead.TYPE_QUARTER },
                            { iconRole: IconCode.NOTE_HEAD_HALF, typeRole: NoteHead.TYPE_HALF },
                            { iconRole: IconCode.NOTE_HEAD_WHOLE, typeRole: NoteHead.TYPE_WHOLE },
                            { iconRole: IconCode.NOTE_HEAD_BREVIS, typeRole: NoteHead.TYPE_BREVIS }
                        ]

                        delegate: FlatRadioButton {

                            ButtonGroup.group: headTypeButtonList.radioButtonGroup

                            checked: root.model && !root.model.noteheadType.isUndefined ? root.model.noteheadType.value === modelData["typeRole"]
                                                                                        : false

                            onToggled: {
                                root.model.noteheadType.value = modelData["typeRole"]
                            }

                            StyledIconLabel {
                                anchors.fill: parent

                                iconCode: modelData["iconRole"]
                            }
                        }
                    }
                }

                InspectorPropertyView {
                    anchors.left: parent.left
                    anchors.right: parent.horizontalCenter
                    anchors.rightMargin: 2

                    titleText: qsTrc("inspector", "Line thickness")
                    propertyItem: root.model ? root.model.lineThickness : null

                    IncrementalPropertyControl {
                        iconMode: iconModeEnum.hidden

                        isIndeterminate: root.model ? root.model.lineThickness.isUndefined : false
                        currentValue: root.model ? root.model.lineThickness.value : 0
                        step: 0.1
                        maxValue: 10
                        minValue: 0.1
                        decimals: 2

                        onValueEdited: { root.model.lineThickness.value = newValue }
                    }
                }
            }
        }
    }
}
