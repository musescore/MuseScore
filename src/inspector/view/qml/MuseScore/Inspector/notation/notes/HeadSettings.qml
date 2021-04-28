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
import QtQuick 2.0
import QtQuick.Controls 2.0
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../common"
import "internal"

FocusableItem {
    id: root

    property QtObject model: null

    implicitHeight: contentColumn.height
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width

        spacing: 16

        CheckBox {
            isIndeterminate: model ? model.isHeadHidden.isUndefined : false
            checked: model && !isIndeterminate ? model.isHeadHidden.value : false
            text: qsTrc("inspector", "Hide notehead")

            onClicked: { model.isHeadHidden.value = !checked }
        }

        InspectorPropertyView {
            titleText: qsTrc("inspector", "Noteheads")
            propertyItem: root.model ? root.model.headGroup : null
            height: 120

            NoteheadsGrid {
                id: noteheadGridView

                noteHeadTypesModel: root.model ? root.model.noteheadTypesModel : null
            }
        }

        InspectorPropertyView {

            titleText: qsTrc("inspector", "Dotted note position")
            propertyItem: root.model ? root.model.dotPosition : null

            RadioButtonGroup {
                id: notePositionButtonList

                height: 30
                width: parent.width

                model: [
                    { iconRole: IconCode.AUTO, typeRole: NoteHead.DOT_POSITION_AUTO },
                    { iconRole: IconCode.DOT_ABOVE_LINE, typeRole: NoteHead.DOT_POSITION_DOWN },
                    { iconRole: IconCode.DOT_BELOW_LINE, typeRole: NoteHead.DOT_POSITION_UP }
                ]

                delegate: FlatRadioButton {

                    ButtonGroup.group: notePositionButtonList.radioButtonGroup

                    checked: root.model && !root.model.dotPosition.isUndefined ? root.model.dotPosition.value === modelData["typeRole"]
                                                                               : false

                    onToggled: {
                        root.model.dotPosition.value = modelData["typeRole"]
                    }

                    StyledIconLabel {
                        iconCode: modelData["iconRole"]
                    }
                }
            }
        }

        ExpandableBlank {
            isExpanded: false

            title: isExpanded ? qsTrc("inspector", "Show less") : qsTrc("inspector", "Show more")

            width: parent.width

            contentItemComponent: Column {
                spacing: 24

                height: implicitHeight
                width: parent.width

                InspectorPropertyView {
                    width: root.width
                    height: implicitHeight

                    titleText: qsTrc("inspector", "Head type (visual only)")
                    propertyItem: root.model ? root.model.headType : null

                    RadioButtonGroup {
                        id: headTypeButtonList

                        height: 30
                        width: parent.width

                        model: [
                            { iconRole: "qrc:/resources/icons/orientation_auto.svg", typeRole: NoteHead.TYPE_AUTO },
                            { iconRole: "qrc:/resources/icons/notehead_quarter.svg", typeRole: NoteHead.TYPE_QUARTER },
                            { iconRole: "qrc:/resources/icons/notehead_half.svg", typeRole: NoteHead.TYPE_HALF },
                            { iconRole: "qrc:/resources/icons/notehead_whole.svg", typeRole: NoteHead.TYPE_WHOLE },
                            { iconRole: "qrc:/resources/icons/notehead_brevis.svg", typeRole: NoteHead.TYPE_BREVIS }
                        ]

                        delegate: FlatRadioButton {

                            ButtonGroup.group: headTypeButtonList.radioButtonGroup

                            checked: root.model && !root.model.headType.isUndefined ? root.model.headType.value === modelData["typeRole"]
                                                                                    : false

                            onToggled: {
                                root.model.headType.value = modelData["typeRole"]
                            }

                            Icon {
                                anchors.fill: parent

                                icon: modelData["iconRole"]
                            }
                        }
                    }
                }

                InspectorPropertyView {
                    width: root.width
                    height: implicitHeight

                    titleText: qsTrc("inspector", "Note direction")
                    propertyItem: root.model ? root.model.headDirection : null

                    RadioButtonGroup {
                        id: noteDirectionButtonList

                        height: 30
                        width: parent.width

                        model: [
                            { iconRole: IconCode.AUTO, typeRole: NoteHead.DIRECTION_H_AUTO },
                            { iconRole: IconCode.ARROW_LEFT, typeRole: NoteHead.DIRECTION_H_LEFT },
                            { iconRole: IconCode.ARROW_RIGHT, typeRole: NoteHead.DIRECTION_H_RIGHT }
                        ]

                        delegate: FlatRadioButton {

                            ButtonGroup.group: noteDirectionButtonList.radioButtonGroup

                            checked: root.model && !root.model.headDirection.isUndefined ? root.model.headDirection.value === modelData["typeRole"]
                                                                                         : false

                            onToggled: {
                                root.model.headDirection.value = modelData["typeRole"]
                            }

                            StyledIconLabel {
                                iconCode: modelData["iconRole"]
                            }
                        }
                    }
                }

                InspectorPropertyView {
                    height: childrenRect.height
                    width: parent.width

                    titleText: qsTrc("inspector", "Notehead offset")
                    propertyItem: model ? model.horizontalOffset : null

                    Item {
                        height: childrenRect.height
                        width: parent.width

                        IncrementalPropertyControl {
                            anchors.left: parent.left
                            anchors.right: parent.horizontalCenter
                            anchors.rightMargin: 4

                            icon: IconCode.HORIZONTAL
                            enabled: model ? !model.isEmpty : false
                            isIndeterminate: model ? model.horizontalOffset.isUndefined : false
                            currentValue: model ? model.horizontalOffset.value : 0

                            onValueEdited: { model.horizontalOffset.value = newValue }
                        }

                        IncrementalPropertyControl {
                            anchors.left: parent.horizontalCenter
                            anchors.leftMargin: 4
                            anchors.right: parent.right

                            icon: IconCode.VERTICAL
                            enabled: model ? !model.isEmpty : false
                            isIndeterminate: model ? model.verticalOffset.isUndefined : false
                            currentValue: model ? model.verticalOffset.value : 0

                            onValueEdited: { model.verticalOffset.value = newValue }
                        }
                    }
                }
            }
        }
    }
}
