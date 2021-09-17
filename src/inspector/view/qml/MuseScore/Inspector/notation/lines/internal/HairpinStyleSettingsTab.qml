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

        Item {
            height: childrenRect.height
            width: parent.width

            CheckBox {
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                isIndeterminate: root.model ? root.model.isNienteCircleVisible.isUndefined : false
                checked: root.model && !isIndeterminate ? root.model.isNienteCircleVisible.value : false
                text: qsTrc("inspector", "Niente circle")

                onClicked: { root.model.isNienteCircleVisible.value = !checked }
            }

            CheckBox {
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                isIndeterminate: root.model ? root.model.isDiagonalLocked.isUndefined : false
                checked: root.model && !isIndeterminate ? root.model.isDiagonalLocked.value : false
                text: qsTrc("inspector", "Allow diagonal")

                onClicked: { root.model.isDiagonalLocked.value = !checked }
            }
        }

        InspectorPropertyView {

            titleText: qsTrc("inspector", "Style")
            propertyItem: root.model ? root.model.lineStyle : null

            RadioButtonGroup {
                id: lineStyleButtonList

                height: 30
                width: parent.width

                model: [
                    { iconRole: IconCode.LINE_NORMAL, typeRole: LineTypes.LINE_STYLE_SOLID },
                    { iconRole: IconCode.LINE_DASHED, typeRole: LineTypes.LINE_STYLE_DASHED },
                    { iconRole: IconCode.LINE_DOTTED, typeRole: LineTypes.LINE_STYLE_DOTTED },
                    { iconRole: IconCode.NONE, textRole: qsTrc("inspector", "Custom"), typeRole: LineTypes.LINE_STYLE_CUSTOM }
                ]

                delegate: FlatRadioButton {
                    ButtonGroup.group: lineStyleButtonList.radioButtonGroup

                    iconCode: modelData["iconRole"]
                    text: modelData["textRole"]

                    checked: root.model && !root.model.lineStyle.isUndefined ? root.model.lineStyle.value === modelData["typeRole"]
                                                                             : false
                    onToggled: {
                        root.model.lineStyle.value = modelData["typeRole"]
                    }
                }
            }
        }

        Item {
            height: childrenRect.height
            width: parent.width

            InspectorPropertyView {
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                visible: root.model ? root.model.dashLineLength.isEnabled : false
                height: visible ? implicitHeight : 0

                titleText: qsTrc("inspector", "Dash")
                propertyItem: root.model ? root.model.dashLineLength : null

                IncrementalPropertyControl {
                    isIndeterminate: root.model ? root.model.dashLineLength.isUndefined : false
                    currentValue: root.model ? root.model.dashLineLength.value : 0
                    step: 0.1
                    maxValue: 10
                    minValue: 0.1
                    decimals: 2

                    onValueEdited: { root.model.dashLineLength.value = newValue }
                }
            }

            InspectorPropertyView {
                anchors.left: parent.horizontalCenter
                anchors.leftMargin: 2
                anchors.right: parent.right

                visible: root.model ? root.model.dashGapLength.isEnabled : false
                height: visible ? implicitHeight : 0

                titleText: qsTrc("inspector", "Gap")
                propertyItem: root.model ? root.model.dashGapLength : null

                IncrementalPropertyControl {
                    isIndeterminate: root.model && enabled ? root.model.dashGapLength.isUndefined : false
                    currentValue: root.model ? root.model.dashGapLength.value : 0
                    step: 0.1
                    maxValue: 10
                    minValue: 0.1
                    decimals: 2

                    onValueEdited: { root.model.dashGapLength.value = newValue }
                }
            }
        }

        SeparatorLine { anchors.margins: -10 }

        Item {
            height: childrenRect.height
            width: parent.width

            InspectorPropertyView {
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                titleText: qsTrc("inspector", "Thickness")
                propertyItem: root.model ? root.model.thickness : null

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

                titleText: qsTrc("inspector", "Height")
                propertyItem: root.model ? root.model.height : null

                IncrementalPropertyControl {
                    isIndeterminate: root.model ? root.model.height.isUndefined : false
                    currentValue: root.model ? root.model.height.value : 0
                    step: 0.1
                    maxValue: 10
                    minValue: 0.1
                    decimals: 2

                    onValueEdited: { root.model.height.value = newValue }
                }
            }
        }

        Item {
            height: childrenRect.height
            width: parent.width

            InspectorPropertyView {
                anchors.left: parent.left
                anchors.right: parent.horizontalCenter
                anchors.rightMargin: 2

                titleText: qsTrc("inspector", "Height (continuing to a new system)")
                propertyItem: root.model ? root.model.continiousHeight : null

                IncrementalPropertyControl {
                    isIndeterminate: root.model ? root.model.continiousHeight.isUndefined : false
                    currentValue: root.model ? root.model.continiousHeight.value : 0
                    step: 0.1
                    maxValue: 10
                    minValue: 0.1
                    decimals: 2

                    onValueEdited: { root.model.continiousHeight.value = newValue }
                }
            }
        }

        InspectorPropertyView {
            titleText: qsTrc("inspector", "Hairpin position")
            propertyItem: root.model ? root.model.placement : null

            RadioButtonGroup {
                id: positionButtonList

                height: 30
                width: parent.width

                model: [
                    { textRole: qsTrc("inspector", "Above"), valueRole: LineTypes.PLACEMENT_TYPE_ABOVE },
                    { textRole: qsTrc("inspector", "Below"), valueRole: LineTypes.PLACEMENT_TYPE_BELOW }
                ]

                delegate: FlatRadioButton {
                    ButtonGroup.group: positionButtonList.radioButtonGroup

                    text: modelData["textRole"]
                    checked: root.model && !root.model.placement.isUndefined ? root.model.placement.value === modelData["valueRole"]
                                                                             : false
                    onToggled: {
                        root.model.placement.value = modelData["valueRole"]
                    }
                }
            }
        }
    }
}

