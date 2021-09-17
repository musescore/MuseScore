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
import QtQuick 2.9
import QtQuick.Controls 2.2
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../../common"

FocusableItem {
    id: root

    property QtObject model: null

    implicitHeight: contentColumn.height
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width

        spacing: 12

        CheckBox {
            isIndeterminate: root.model ? root.model.isLineVisible.isUndefined : false
            checked: root.model && !isIndeterminate ? root.model.isLineVisible.value : false
            text: qsTrc("inspector", "Show line")

            onClicked: { root.model.isLineVisible.value = !checked }
        }

        InspectorPropertyView {
            width: parent.width

            titleText: qsTrc("inspector", "Line type")
            propertyItem: root.model ? root.model.endHookType : null

            enabled: root.model && root.model.endHookType.isEnabled

            RadioButtonGroup {
                id: lineTypeButtonList

                height: 30
                width: parent.width

                model: [
                    { iconRole: IconCode.LINE_NORMAL, typeRole: CrescendoTypes.HOOK_TYPE_NONE },
                    { iconRole: IconCode.LINE_WITH_END_HOOK, typeRole: CrescendoTypes.HOOK_TYPE_90 },
                    { iconRole: IconCode.LINE_WITH_ANGLED_END_HOOK, typeRole: CrescendoTypes.HOOK_TYPE_45 },
                    { iconRole: IconCode.LINE_WITH_T_LIKE_END_HOOK, typeRole: CrescendoTypes.HOOK_TYPE_T_LIKE },
                ]

                delegate: FlatRadioButton {
                    ButtonGroup.group: lineTypeButtonList.radioButtonGroup

                    iconCode: modelData["iconRole"]
                    checked: root.model && !root.model.endHookType.isUndefined ? root.model.endHookType.value === modelData["typeRole"]
                                                                               : false
                    onToggled: {
                        root.model.endHookType.value = modelData["typeRole"]
                    }
                }
            }
        }

        SeparatorLine { anchors.margins: -10 }

        LineStyleSection {
            lineStyle: root.model ? root.model.lineStyle : null
            dashLineLength: root.model ? root.model.dashLineLength : null
            dashGapLength: root.model ? root.model.dashGapLength : null
        }

        ExpandableBlank {
            isExpanded: false

            title: isExpanded ? qsTrc("inspector", "Show less") : qsTrc("inspector", "Show more")

            width: parent.width

            contentItemComponent: Column {
                height: implicitHeight
                width: root.width

                spacing: 16

                Item {
                    height: childrenRect.height
                    width: parent.width

                    InspectorPropertyView {
                        anchors.left: parent.left
                        anchors.right: parent.horizontalCenter
                        anchors.rightMargin: 2

                        titleText: qsTrc("inspector", "Thickness")
                        propertyItem: root.model ? root.model.thickness : null

                        enabled: root.model && root.model.thickness.isEnabled

                        IncrementalPropertyControl {
                            isIndeterminate: root.model ? root.model.thickness.isUndefined : false
                            currentValue: root.model ? root.model.thickness.value : 0
                            step: 0.1
                            maxValue: 10
                            minValue: 0.1
                            decimals: 2

                            onValueEdited: { root.model.thickness.value = newValue }
                        }
                    }

                    InspectorPropertyView {
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 2
                        anchors.right: parent.right

                        titleText: qsTrc("inspector", "Hook height")
                        propertyItem: root.model ? root.model.hookHeight : null

                        enabled: root.model && root.model.hookHeight.isEnabled

                        IncrementalPropertyControl {
                            isIndeterminate: root.model ? root.model.hookHeight.isUndefined : false
                            currentValue: root.model ? root.model.hookHeight.value : 0
                            step: 0.1
                            maxValue: 10
                            minValue: 0.1
                            decimals: 2

                            onValueEdited: { root.model.hookHeight.value = newValue }
                        }
                    }
                }

                PlacementSection {
                    propertyItem: root.model ? root.model.placement : null
                }
            }
        }
    }
}

