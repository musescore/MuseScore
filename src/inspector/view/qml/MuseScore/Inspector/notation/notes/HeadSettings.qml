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
            isIndeterminate: root.model ? root.model.isHeadHidden.isUndefined : false
            checked: root.model && !isIndeterminate ? root.model.isHeadHidden.value : false
            text: qsTrc("inspector", "Hide notehead")

            onClicked: { root.model.isHeadHidden.value = !checked }
        }

        InspectorPropertyView {
            titleText: qsTrc("inspector", "Notehead group")
            propertyItem: root.model ? root.model.headGroup : null

            NoteheadsGrid {
                id: noteheadGridView
                noteHeadGroupsModel: root.model ? root.model.noteheadGroupsModel : null
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
                    { iconRole: IconCode.NONE, textRole: qsTrc("inspector", "Auto"), typeRole: NoteHead.DOT_POSITION_AUTO },
                    { iconRole: IconCode.DOT_ABOVE_LINE, typeRole: NoteHead.DOT_POSITION_DOWN },
                    { iconRole: IconCode.DOT_BELOW_LINE, typeRole: NoteHead.DOT_POSITION_UP }
                ]

                delegate: FlatRadioButton {
                    ButtonGroup.group: notePositionButtonList.radioButtonGroup

                    iconCode: modelData["iconRole"]
                    text: modelData["textRole"]

                    checked: root.model && !root.model.dotPosition.isUndefined ? root.model.dotPosition.value === modelData["typeRole"]
                                                                               : false
                    onToggled: {
                        root.model.dotPosition.value = modelData["typeRole"]
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

                NoteHeadTypeSelector {
                    propertyItem: root.model ? root.model.headType : null
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
                            { iconRole: IconCode.NONE, textRole: qsTrc("inspector", "Auto"), typeRole: NoteHead.DIRECTION_H_AUTO },
                            { iconRole: IconCode.ARROW_LEFT, typeRole: NoteHead.DIRECTION_H_LEFT },
                            { iconRole: IconCode.ARROW_RIGHT, typeRole: NoteHead.DIRECTION_H_RIGHT }
                        ]

                        delegate: FlatRadioButton {
                            ButtonGroup.group: noteDirectionButtonList.radioButtonGroup

                            iconCode: modelData["iconRole"]
                            text: modelData["textRole"]

                            checked: root.model && !root.model.headDirection.isUndefined ? root.model.headDirection.value === modelData["typeRole"]
                                                                                         : false
                            onToggled: {
                                root.model.headDirection.value = modelData["typeRole"]
                            }
                        }
                    }
                }

                InspectorPropertyView {
                    height: childrenRect.height
                    width: parent.width

                    titleText: qsTrc("inspector", "Notehead offset")
                    propertyItem: root.model ? root.model.horizontalOffset : null

                    Item {
                        height: childrenRect.height
                        width: parent.width

                        IncrementalPropertyControl {
                            anchors.left: parent.left
                            anchors.right: parent.horizontalCenter
                            anchors.rightMargin: 4

                            icon: IconCode.HORIZONTAL
                            enabled: root.model ? !root.model.isEmpty : false
                            isIndeterminate: root.model ? root.model.horizontalOffset.isUndefined : false
                            currentValue: root.model ? root.model.horizontalOffset.value : 0

                            onValueEdited: { root.model.horizontalOffset.value = newValue }
                        }

                        IncrementalPropertyControl {
                            anchors.left: parent.horizontalCenter
                            anchors.leftMargin: 4
                            anchors.right: parent.right

                            icon: IconCode.VERTICAL
                            enabled: root.model ? !root.model.isEmpty : false
                            isIndeterminate: root.model ? root.model.verticalOffset.isUndefined : false
                            currentValue: root.model ? root.model.verticalOffset.value : 0

                            onValueEdited: { root.model.verticalOffset.value = newValue }
                        }
                    }
                }
            }
        }
    }
}
