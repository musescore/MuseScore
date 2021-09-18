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
            isIndeterminate: root.stemModel && root.beamModel ? root.stemModel.isStemHidden.isUndefined || root.beamModel.isBeamHidden.isUndefined : false
            checked: root.stemModel && !isIndeterminate && root.beamModel ? root.stemModel.isStemHidden.value && root.beamModel.isBeamHidden.value : false
            text: qsTrc("inspector", "Hide stem (also hides beam)")

            onClicked: {
                var isHidden = !checked
                root.stemModel.isStemHidden.value = isHidden
                root.beamModel.isBeamHidden.value = isHidden
            }
        }

        FlatRadioButtonGroupPropertyView {
            titleText: qsTrc("inspector", "Stem direction")
            propertyItem: root.stemModel ? root.stemModel.stemDirection : null

            model: [
                { text: qsTrc("inspector", "Auto"), value: DirectionTypes.VERTICAL_AUTO },
                { iconCode: IconCode.ARROW_DOWN, value: DirectionTypes.VERTICAL_DOWN },
                { iconCode: IconCode.ARROW_UP, value: DirectionTypes.VERTICAL_UP }
            ]
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
                        propertyItem: root.stemModel ? root.stemModel.thickness : null

                        IncrementalPropertyControl {
                            enabled: root.stemModel ? !root.stemModel.isEmpty : false
                            isIndeterminate: root.stemModel ? root.stemModel.thickness.isUndefined : false
                            currentValue: root.stemModel ? root.stemModel.thickness.value : 0

                            maxValue: 4
                            minValue: 0.01
                            step: 0.01

                            onValueEdited: { root.stemModel.thickness.value = newValue }
                        }
                    }

                    InspectorPropertyView {
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 2
                        anchors.right: parent.right

                        titleText: qsTrc("inspector", "Length")
                        propertyItem: root.stemModel ? root.stemModel.length : null

                        IncrementalPropertyControl {
                            enabled: root.stemModel ? !root.stemModel.isEmpty : false
                            isIndeterminate: root.stemModel ? root.stemModel.length.isUndefined : false
                            currentValue: root.stemModel ? root.stemModel.length.value : 0

                            maxValue: 10
                            minValue: 0.01

                            onValueEdited: { root.stemModel.length.value = newValue }
                        }
                    }
                }

                InspectorPropertyView {
                    height: implicitHeight

                    titleText: qsTrc("inspector", "Stem offset")
                    propertyItem: root.stemModel ? root.stemModel.horizontalOffset : null

                    Item {
                        height: childrenRect.height
                        width: parent.width

                        IncrementalPropertyControl {
                            anchors.left: parent.left
                            anchors.right: parent.horizontalCenter
                            anchors.rightMargin: 4

                            icon: IconCode.HORIZONTAL
                            enabled: root.stemModel ? !root.stemModel.isEmpty : false
                            isIndeterminate: root.stemModel ? root.stemModel.horizontalOffset.isUndefined : false
                            currentValue: root.stemModel ? root.stemModel.horizontalOffset.value : 0

                            onValueEdited: { root.stemModel.horizontalOffset.value = newValue }
                        }

                        IncrementalPropertyControl {
                            anchors.left: parent.horizontalCenter
                            anchors.leftMargin: 4
                            anchors.right: parent.right

                            icon: IconCode.VERTICAL
                            enabled: root.stemModel ? !root.stemModel.isEmpty : false
                            isIndeterminate: root.stemModel ? root.stemModel.verticalOffset.isUndefined : false
                            currentValue: root.stemModel ? root.stemModel.verticalOffset.value : 0

                            onValueEdited: { root.stemModel.verticalOffset.value = newValue }
                        }
                    }
                }

                InspectorPropertyView {
                    height: childrenRect.height

                    titleText: qsTrc("inspector", "Flag offset")
                    propertyItem: root.hookModel ? root.hookModel.horizontalOffset : null

                    Item {
                        height: childrenRect.height
                        width: parent.width

                        IncrementalPropertyControl {
                            anchors.left: parent.left
                            anchors.right: parent.horizontalCenter
                            anchors.rightMargin: 4

                            enabled: root.hookModel ? !root.hookModel.isEmpty : false
                            isIndeterminate: root.hookModel ? root.hookModel.horizontalOffset.isUndefined : false
                            icon: IconCode.HORIZONTAL
                            currentValue: root.hookModel ? root.hookModel.horizontalOffset.value : 0.00

                            onValueEdited: { root.hookModel.horizontalOffset.value = newValue }
                        }

                        IncrementalPropertyControl {
                            anchors.left: parent.horizontalCenter
                            anchors.leftMargin: 4
                            anchors.right: parent.right

                            enabled: root.hookModel ? !root.hookModel.isEmpty : false
                            isIndeterminate: root.hookModel ? root.hookModel.verticalOffset.isUndefined : false
                            icon: IconCode.VERTICAL
                            currentValue: root.hookModel ? root.hookModel.verticalOffset.value : 0.00

                            onValueEdited: { root.hookModel.verticalOffset.value = newValue }
                        }
                    }
                }
            }
        }
    }
}
