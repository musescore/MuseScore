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

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.Inspector 1.0

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

        spacing: 12

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

        Column {
            width: parent.width
            height: childrenRect.height
            spacing: 8

            StyledTextLabel {
                width: parent.width
                text: qsTrc("inspector", "Flag style")
                horizontalAlignment: Text.AlignLeft
            }

            RadioButtonGroup {
                width: parent.width
                height: 70

                model: [
                    { iconCode: IconCode.NOTEFLAGS_TRADITIONAL, text: qsTrc("inspector", "Traditional", "Note flags"), value: false },
                    { iconCode: IconCode.NOTEFLAGS_STRAIGHT, text: qsTrc("inspector", "Straight", "Note flags"), value: true }
                ]

                delegate: FlatRadioButton {
                    height: 70

                    Column {
                        anchors.centerIn: parent
                        height: childrenRect.height
                        spacing: 8

                        StyledIconLabel {
                            anchors.horizontalCenter: parent.horizontalCenter
                            iconCode: modelData.iconCode
                            font.pixelSize: 32
                        }

                        StyledTextLabel {
                            anchors.horizontalCenter: parent.horizontalCenter
                            text: modelData.text
                        }
                    }

                    checked: root.stemModel && root.stemModel.useStraightNoteFlags === modelData.value
                    onToggled: {
                        if (root.stemModel) {
                            root.stemModel.useStraightNoteFlags = modelData.value
                        }
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

                spacing: 12

                Item {
                    height: childrenRect.height
                    width: parent.width

                    SpinBoxPropertyView {
                        anchors.left: parent.left
                        anchors.right: parent.horizontalCenter
                        anchors.rightMargin: 2

                        titleText: qsTrc("inspector", "Thickness")
                        propertyItem: root.stemModel ? root.stemModel.thickness : null

                        maxValue: 4
                        minValue: 0.01
                        step: 0.01
                    }

                    SpinBoxPropertyView {
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 2
                        anchors.right: parent.right

                        titleText: qsTrc("inspector", "Length")
                        propertyItem: root.stemModel ? root.stemModel.length : null

                        maxValue: 10
                        minValue: -10
                    }
                }

                OffsetSection {
                    titleText: qsTrc("inspector", "Stem offset")
                    horizontalOffset: root.stemModel ? root.stemModel.horizontalOffset : null
                    verticalOffset: root.stemModel ? root.stemModel.verticalOffset : null
                }

                OffsetSection {
                    titleText: qsTrc("inspector", "Flag offset")
                    horizontalOffset: root.hookModel ? root.hookModel.horizontalOffset : null
                    verticalOffset: root.hookModel ? root.hookModel.verticalOffset : null
                }
            }
        }
    }
}
