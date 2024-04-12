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

import MuseScore.Inspector 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0

import "../beams"
import "../../common"

FocusableItem {
    id: root

    property QtObject model: null
    readonly property QtObject beamModesModel: model ? model.beamModesModel : null

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    implicitHeight: contentColumn.height
    width: parent.width

    Column {
        id: contentColumn

        width: parent.width
        spacing: 12

        BeamTypeSelector {
            id: beamTypeSection
            titleText: qsTrc("inspector", "Beam type")
            propertyItem: root.beamModesModel ? root.beamModesModel.mode : null
            enabled: root.beamModesModel && !root.beamModesModel.isEmpty

            navigationPanel: root.navigationPanel
            navigationRowStart: root.navigationRowStart
        }

        Column {
            spacing: 12

            height: implicitHeight
            width: parent.width

            enabled: root.model ? !root.model.isEmpty : false

            Column {
                id: featheringControlsColumn

                spacing: 8

                height: implicitHeight
                width: parent.width

                visible: root.beamModesModel ? root.beamModesModel.isFeatheringAvailable.isUndefined || root.beamModesModel.isFeatheringAvailable.value
                                             : false

                StyledTextLabel {
                    id: featheredBeamsLabel
                    width: parent.width
                    text: qsTrc("inspector", "Feathered beams")
                    horizontalAlignment: Text.AlignLeft
                }

                RadioButtonGroup {
                    id: featheredBeamsButtonList

                    property int navigationRowStart: beamTypeSection.navigationRowEnd + 1
                    property int navigationRowEnd: navigationRowStart + model.length

                    height: 30
                    width: parent.width

                    model: [
                        { text: qsTrc("inspector", "None"), value: Beam.FEATHERING_NONE, title: qsTrc("inspector", "None") },
                        { iconCode: IconCode.BEAM_FEATHERED_DECELERATE, value: Beam.FEATHERED_DECELERATE, title: qsTrc("inspector", "Decelerate") },
                        { iconCode: IconCode.BEAM_FEATHERED_ACCELERATE, value: Beam.FEATHERED_ACCELERATE, title: qsTrc("inspector", "Accelerate") }
                    ]

                    delegate: FlatRadioButton {
                        iconCode: modelData["iconCode"] ?? IconCode.NONE
                        text: modelData["text"] ?? ""

                        navigation.name: text
                        navigation.panel: root.navigationPanel
                        navigation.row: featheredBeamsButtonList.navigationRowStart + index
                        navigation.accessible.name: featheredBeamsLabel.text + " " + text

                        checked: root.beamModesModel && !(root.model.featheringHeightLeft.isUndefined || root.model.featheringHeightRight.isUndefined)
                                 ? root.model.featheringMode === modelData["value"]
                                 : false
                        onToggled: {
                            root.model.featheringMode = modelData["value"]
                        }
                    }
                }

                Item {
                    height: childrenRect.height
                    width: parent.width

                    visible: root.model && root.model.isFeatheringHeightChangingAllowed

                    SpinBoxPropertyView {
                        id: featheringLeftSection
                        anchors.left: parent.left
                        anchors.right: parent.horizontalCenter
                        anchors.rightMargin: 4

                        titleText: qsTrc("inspector", "Feathering left")
                        propertyItem: root.model ? root.model.featheringHeightLeft : null
                        enabled: root.beamModesModel ? root.beamModesModel.isFeatheringAvailable : false

                        icon: IconCode.BEAM_FEATHERING_LEFT_HEIGHT
                        maxValue: 4
                        minValue: 0
                        step: 0.1

                        navigationName: "FeatheringLeft"
                        navigationPanel: root.navigationPanel
                        navigationRowStart: featheredBeamsButtonList.navigationRowEnd + 1
                    }

                    SpinBoxPropertyView {
                        id: featheringRightSection
                        anchors.left: parent.horizontalCenter
                        anchors.leftMargin: 4
                        anchors.right: parent.right

                        titleText: qsTrc("inspector", "Feathering right")
                        propertyItem: root.model ? root.model.featheringHeightRight : null
                        enabled: root.beamModesModel ? root.beamModesModel.isFeatheringAvailable : false

                        icon: IconCode.BEAM_FEATHERING_RIGHT_HEIGHT
                        iconMode: IncrementalPropertyControl.Right
                        maxValue: 4
                        minValue: 0
                        step: 0.1

                        navigationName: "FeatheringRight"
                        navigationPanel: root.navigationPanel
                        navigationRowStart: featheringLeftSection.navigationRowEnd + 1
                    }
                }
            }

            DirectionSection {
                id: beamDirection
                visible: root.model ? !(root.model.isCrossStaffMoveAvailable) : true

                titleText: qsTrc("inspector", "Beam direction")
                propertyItem: root.model ? root.model.stemDirection : null

                navigationPanel: root.navigationPanel
                navigationRowStart: featheringRightSection.navigationRowEnd + 1
            }

            InspectorPropertyView {
                id: crossStaffMove
                visible: root.model ? root.model.isCrossStaffMoveAvailable : false

                navigationName: "Move cross-staff beam"
                navigationPanel: root.navigationPanel
                navigationRowStart: beamDirection.navigationRowEnd + 1
                navigationRowEnd: crossBeamDown.navigation.row

                titleText: qsTrc("inspector", "Move cross-staff beam")
                propertyItem: root.model ? root.model.crossStaffMove : null

                Row {
                    spacing: 4
                    width: parent.width

                    FlatRadioButton {
                        id: crossBeamUp
                        width: 0.5 * parent.width - 2
                        iconCode: IconCode.ARROW_UP
                        checked: root.model ? root.model.crossStaffMove.value < 0 : false
                        onClicked: {
                            root.model.crossStaffMove.value -= 1
                        }

                        navigation.panel: root.navigationPanel
                        navigation.row: crossStaffMove.navigationRowStart + 1
                    }

                    FlatRadioButton {
                        id: crossBeamDown
                        width: 0.5 * parent.width - 2
                        iconCode: IconCode.ARROW_DOWN
                        checked: root.model ? root.model.crossStaffMove.value > 0 : false
                        onClicked: {
                            root.model.crossStaffMove.value += 1
                        }

                        navigation.panel: root.navigationPanel
                        navigation.row: crossBeamUp.navigation.row + 1
                    }
                }
            }

            PropertyCheckBox {
                id: forceHorizontalButton
                width: parent.width

                text: qsTrc("inspector", "Force horizontal")
                propertyItem: root.model ? root.model.forceHorizontal : null

                navigation.name: "ForceHorizontal"
                navigation.panel: root.navigationPanel
                navigation.row: crossStaffMove.navigationRowEnd + 1
            }

            ExpandableBlank {
                id: showItem
                isExpanded: false

                title: isExpanded ? qsTrc("inspector", "Show less") : qsTrc("inspector", "Show more")

                width: parent.width

                navigation.panel: root.navigationPanel
                navigation.row: forceHorizontalButton.navigation.row + 1

                contentItemComponent: Column {
                    height: implicitHeight
                    width: root.width

                    spacing: 12

                    InspectorPropertyView {
                        id: beamHeight
                        titleText: qsTrc("inspector", "Beam height")
                        propertyItem: root.model ? root.model.customPositioned : null

                        navigationName: "Beam height"
                        navigationPanel: root.navigationPanel
                        navigationRowStart: showItem.navigation.row + 1
                        navigationRowEnd: beamHeightRightControl.navigation.row

                        Item {
                            height: childrenRect.height
                            width: parent.width

                            IncrementalPropertyControl {
                                id: beamHeightLeftControl

                                anchors.left: parent.left
                                anchors.right: lockButton.left
                                anchors.rightMargin: 6

                                icon: IconCode.BEAM_HEIGHT_LEFT
                                isIndeterminate: root.model ? root.model.beamHeightLeft.isUndefined : false
                                currentValue: root.model ? root.model.beamHeightLeft.value : 0

                                navigation.name: "BeamHeightLeftControl"
                                navigation.panel: root.navigationPanel
                                navigation.row: beamHeight.navigationRowStart + 1
                                navigation.accessible.name: beamHeight.titleText + " " + qsTrc("inspector", "Left") + " " + currentValue

                                onValueEditingFinished: function(newValue) {
                                    root.model.beamHeightLeft.value = newValue
                                }
                            }

                            FlatToggleButton {
                                id: lockButton

                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.verticalCenter: beamHeightLeftControl.verticalCenter

                                height: 20
                                width: 20

                                icon: checked ? IconCode.LOCK_CLOSED : IconCode.LOCK_OPEN

                                checked: root.model ? root.model.isBeamHeightLocked : false

                                navigation.name: "Lock beam height"
                                navigation.panel: root.navigationPanel
                                navigation.row: beamHeightLeftControl.navigation.row + 1
                                navigation.accessible.name: qsTrc("inspector", "Lock")

                                onToggled: {
                                    root.model.isBeamHeightLocked = !root.model.isBeamHeightLocked
                                }
                            }

                            IncrementalPropertyControl {
                                id: beamHeightRightControl
                                anchors.left: lockButton.right
                                anchors.leftMargin: 6
                                anchors.right: parent.right

                                icon: IconCode.BEAM_HEIGHT_RIGHT
                                iconMode: IncrementalPropertyControl.Right
                                isIndeterminate: root.model ? root.model.beamHeightRight.isUndefined : false
                                currentValue: root.model ? root.model.beamHeightRight.value : 0

                                navigation.name: "BeamHeightRightControl"
                                navigation.panel: root.navigationPanel
                                navigation.row: lockButton.navigation.row + 1
                                navigation.accessible.name: beamHeight.titleText + " " + qsTrc("inspector", "Right") + " " + currentValue

                                onValueEditingFinished: function(newValue) {
                                    root.model.beamHeightRight.value = newValue
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
