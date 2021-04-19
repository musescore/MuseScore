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


FocusableItem {
    id: root

    //@note Current design assumes that stems and hooks should be represented at the same tab,
    //      but semantically it's different things, so they should have different models
    property QtObject stemModel: null
    property QtObject hookModel: null
    property QtObject beamModel: null

    implicitHeight: contentColumn.height
    width: parent.width

    Column {
        id: contentColumn

        height: implicitHeight
        width: root.width

        spacing: 16

        CheckBox {
            isIndeterminate: stemModel && beamModel ? stemModel.isStemHidden.isUndefined || beamModel.isBeamHidden.isUndefined : false
            checked: stemModel && !isIndeterminate && beamModel ? stemModel.isStemHidden.value && beamModel.isBeamHidden.value : false
            text: qsTrc("inspector", "Hide stem (also hides beam)")

            onClicked: {
                var isHidden = !checked
                stemModel.isStemHidden.value = isHidden
                beamModel.isBeamHidden.value = isHidden
            }
        }

        InspectorPropertyView {

            titleText: qsTrc("inspector", "Stem direction")
            propertyItem: root.stemModel ? root.stemModel.stemDirection : null

            RadioButtonGroup {
                id: radioButtonList

                height: 30
                width: parent.width

                model: [
                    { iconRole: IconCode.AUTO, typeRole: DirectionTypes.VERTICAL_AUTO },
                    { iconRole: IconCode.ARROW_DOWN, typeRole: DirectionTypes.VERTICAL_DOWN },
                    { iconRole: IconCode.ARROW_UP, typeRole: DirectionTypes.VERTICAL_UP }
                ]

                delegate: FlatRadioButton {

                    ButtonGroup.group: radioButtonList.radioButtonGroup

                    checked: root.stemModel && !root.stemModel.stemDirection.isUndefined ? root.stemModel.stemDirection.value === modelData["typeRole"]
                                                                                         : false

                    onToggled: {
                        root.stemModel.stemDirection.value = modelData["typeRole"]
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
                        propertyItem: stemModel ? stemModel.thickness : null

                        IncrementalPropertyControl {
                            enabled: stemModel ? !stemModel.isEmpty : false
                            isIndeterminate: stemModel ? stemModel.thickness.isUndefined : false
                            currentValue: stemModel ? stemModel.thickness.value : 0
                            iconMode: iconModeEnum.hidden

                            maxValue: 4
                            minValue: 0.01
                            step: 0.01

                            onValueEdited: { stemModel.thickness.value = newValue }
                        }
                    }

                    InspectorPropertyView {
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 2
                        anchors.right: parent.right

                        titleText: qsTrc("inspector", "Length")
                        propertyItem: stemModel ? stemModel.length : null

                        IncrementalPropertyControl {
                            enabled: stemModel ? !stemModel.isEmpty : false
                            isIndeterminate: stemModel ? stemModel.length.isUndefined : false
                            currentValue: stemModel ? stemModel.length.value : 0
                            iconMode: iconModeEnum.hidden

                            maxValue: 10
                            minValue: 0.01

                            onValueEdited: { stemModel.length.value = newValue }
                        }
                    }
                }

                InspectorPropertyView {
                    height: implicitHeight

                    titleText: qsTrc("inspector", "Stem offset")
                    propertyItem: stemModel ? stemModel.horizontalOffset : null

                    Item {
                        height: childrenRect.height
                        width: parent.width

                        IncrementalPropertyControl {
                            anchors.left: parent.left
                            anchors.right: parent.horizontalCenter
                            anchors.rightMargin: 4

                            icon: IconCode.HORIZONTAL
                            enabled: stemModel ? !stemModel.isEmpty : false
                            isIndeterminate: stemModel ? stemModel.horizontalOffset.isUndefined : false
                            currentValue: stemModel ? stemModel.horizontalOffset.value : 0

                            onValueEdited: { stemModel.horizontalOffset.value = newValue }
                        }

                        IncrementalPropertyControl {
                            anchors.left: parent.horizontalCenter
                            anchors.leftMargin: 4
                            anchors.right: parent.right

                            icon: IconCode.VERTICAL
                            enabled: stemModel ? !stemModel.isEmpty : false
                            isIndeterminate: stemModel ? stemModel.verticalOffset.isUndefined : false
                            currentValue: stemModel ? stemModel.verticalOffset.value : 0

                            onValueEdited: { stemModel.verticalOffset.value = newValue }
                        }
                    }
                }

                InspectorPropertyView {
                    height: childrenRect.height

                    titleText: qsTrc("inspector", "Flag offset")
                    propertyItem: hookModel ? hookModel.horizontalOffset : null

                    Item {
                        height: childrenRect.height
                        width: parent.width

                        IncrementalPropertyControl {
                            anchors.left: parent.left
                            anchors.right: parent.horizontalCenter
                            anchors.rightMargin: 4

                            enabled: hookModel ? !hookModel.isEmpty : false
                            isIndeterminate: hookModel ? hookModel.horizontalOffset.isUndefined : false
                            icon: IconCode.HORIZONTAL
                            currentValue: hookModel ? hookModel.horizontalOffset.value : 0.00

                            onValueEdited: { hookModel.horizontalOffset.value = newValue }
                        }

                        IncrementalPropertyControl {
                            anchors.left: parent.horizontalCenter
                            anchors.leftMargin: 4
                            anchors.right: parent.right

                            enabled: hookModel ? !hookModel.isEmpty : false
                            isIndeterminate: hookModel ? hookModel.verticalOffset.isUndefined : false
                            icon: IconCode.VERTICAL
                            currentValue: hookModel ? hookModel.verticalOffset.value : 0.00

                            onValueEdited: { hookModel.verticalOffset.value = newValue }
                        }
                    }
                }
            }
        }
    }
}
