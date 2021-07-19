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

Column {
    id: root

    property QtObject model: null

    objectName: "PedalSettings"

    spacing: 12

    InspectorPropertyView {

        titleText: qsTrc("inspector", "Line type")
        propertyItem: root.model ? root.model.hookType : null

        RadioButtonGroup {
            id: lineTypeButtonList

            height: 30
            width: parent.width

            model: [
                { iconRole: IconCode.LINE_NORMAL, typeRole: PedalTypes.HOOK_TYPE_NONE },
                { iconRole: IconCode.LINE_WITH_END_HOOK, typeRole: PedalTypes.HOOK_TYPE_RIGHT_ANGLE },
                { iconRole: IconCode.LINE_WITH_ANGLED_END_HOOK, typeRole: PedalTypes.HOOK_TYPE_ACUTE_ANGLE },
                { iconRole: IconCode.LINE_PEDAL_STAR_ENDING, typeRole: PedalTypes.HOOK_TYPE_STAR }
            ]

            delegate: FlatRadioButton {

                ButtonGroup.group: lineTypeButtonList.radioButtonGroup

                checked: root.model && !root.model.hookType.isUndefined ? root.model.hookType.value === modelData["typeRole"]
                                                                        : false

                onToggled: {
                    root.model.hookType.value = modelData["typeRole"]
                }

                StyledIconLabel {
                    iconCode: modelData["iconRole"]
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

            titleText: qsTrc("inspector", "Thickness")
            propertyItem: root.model ? root.model.thickness : null

            IncrementalPropertyControl {
                id: thicknessControl
                iconMode: iconModeEnum.hidden

                isIndeterminate: root.model ? root.model.thickness.isUndefined : false
                currentValue: root.model ? root.model.thickness.value : 0

                onValueEdited: { root.model.thickness.value = newValue }
            }
        }

        InspectorPropertyView {
            anchors.left: parent.horizontalCenter
            anchors.leftMargin: 2
            anchors.right: parent.right

            titleText: qsTrc("inspector", "Hook height")
            propertyItem: root.model ? root.model.hookHeight : null

            IncrementalPropertyControl {
                id: hookHeightControl

                iconMode: iconModeEnum.hidden

                enabled: root.model ? root.model.hookHeight.isEnabled : false
                isIndeterminate: root.model && enabled ? root.model.hookHeight.isUndefined : false
                currentValue: root.model ? root.model.hookHeight.value : 0

                onValueEdited: { root.model.hookHeight.value = newValue }
            }
        }
    }

    CheckBox {
        id: showBothSideHookCheckbox

        /*isIndeterminate: model ? model.isDefaultTempoForced.isUndefined : false
            checked: model && !isIndeterminate ? model.isDefaultTempoForced.value : false*/
        text: qsTrc("inspector", "Show hook on both ends")

        //onClicked: { model.isDefaultTempoForced.value = !checked }
    }

    SeparatorLine { anchors.margins: -10 }

    RadioButtonGroup {
        id: lineStyleButtonList

        height: 30
        width: parent.width

        model: [
            { iconRole: IconCode.LINE_NORMAL, typeRole: PedalTypes.LINE_STYLE_SOLID },
            { iconRole: IconCode.LINE_DASHED, typeRole: PedalTypes.LINE_STYLE_DASHED },
            { iconRole: IconCode.LINE_DOTTED, typeRole: PedalTypes.LINE_STYLE_DOTTED },
            { iconRole: IconCode.CUSTOM, typeRole: PedalTypes.LINE_STYLE_CUSTOM }
        ]

        delegate: FlatRadioButton {

            ButtonGroup.group: lineStyleButtonList.radioButtonGroup

            checked: root.model && !root.model.lineStyle.isUndefined ? root.model.lineStyle.value === modelData["typeRole"]
                                                                     : false

            onToggled: {
                root.model.lineStyle.value = modelData["typeRole"]
            }

            StyledIconLabel {
                iconCode: modelData["iconRole"]
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

            titleText: qsTrc("inspector", "Dash")
            propertyItem: root.model ? root.model.dashLineLength : null

            IncrementalPropertyControl {
                id: dashControl
                iconMode: iconModeEnum.hidden

                isIndeterminate: root.model ? root.model.dashLineLength.isUndefined : false
                currentValue: root.model ? root.model.dashLineLength.value : 0

                onValueEdited: { root.model.dashLineLength.value = newValue }
            }
        }

        InspectorPropertyView {
            anchors.left: parent.horizontalCenter
            anchors.leftMargin: 2
            anchors.right: parent.right

            titleText: qsTrc("inspector", "Gap")
            propertyItem: root.model ? root.model.dashGapLength : null

            IncrementalPropertyControl {
                id: gapControl

                iconMode: iconModeEnum.hidden

                enabled: root.model ? root.model.dashGapLength.isEnabled : false
                isIndeterminate: root.model && enabled ? root.model.dashGapLength.isUndefined : false
                currentValue: root.model ? root.model.dashGapLength.value : 0

                onValueEdited: { root.model.dashGapLength.value = newValue }
            }
        }
    }

    RadioButtonGroup {
        id: pedalPositionButtonList

        height: 30
        width: parent.width

        model: [
            { textRole: qsTrc("inspector", "Above"), valueRole: PedalTypes.PLACEMENT_TYPE_ABOVE },
            { textRole: qsTrc("inspector", "Below"), valueRole: PedalTypes.PLACEMENT_TYPE_BELOW }
        ]

        delegate: FlatRadioButton {

            ButtonGroup.group: pedalPositionButtonList.radioButtonGroup

            checked: root.model && !root.model.placement.isUndefined ? root.model.placement.value === modelData["valueRole"]
                                                                     : false
            onToggled: {
                root.model.placement.value = modelData["valueRole"]
            }

            StyledTextLabel {
                text: modelData["textRole"]

                elide: Text.ElideRight
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
            }
        }
    }
}
