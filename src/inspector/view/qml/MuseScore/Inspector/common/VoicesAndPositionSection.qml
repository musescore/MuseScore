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
import QtQuick.Layouts

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import MuseScore.Inspector 1.0

import "../common"

Column {
    id: root

    width: parent.width

    required property QtObject model

    property NavigationPanel navigationPanel: null
    property int navigationRowStart: 1

    spacing: 12

    InspectorPropertyView {
        id: voiceAssignmentSection

        titleText: qsTrc("inspector", "Voice assignment")

        propertyItem: root.model ? root.model.voiceAssignment : null

        onRequestResetToDefault: {
            if (root.model) {
                propertyItem.resetToDefault()
            }
        }

        navigationPanel: root.navigationPanel
        navigationRowStart: root.navigationRowStart
        navigationRowEnd: individualVoicesSection.navigationRowEnd

        RowLayout {
            width: parent.width
            spacing: individualVoicesSection.spacing

            FlatButton {
                id: allVoicesButton

                readonly property bool isMenuButton: root.model && root.model.isMultiStaffInstrument

                Layout.preferredWidth: (parent.width - 2 * parent.spacing) / 3 // One third

                text: qsTrc("inspector", "All")
                accentButton: voiceAssignmentSection.propertyItem
                              && (voiceAssignmentSection.propertyItem.value === VoiceTypes.VOICE_ALL_IN_INSTRUMENT
                                  || voiceAssignmentSection.propertyItem.value === VoiceTypes.VOICE_ALL_IN_STAFF)
                backgroundRadius: 2 // match FlatRadioButton

                navigation.panel: root.navigationPanel
                navigation.row: voiceAssignmentSection.navigationRowStart + 1

                mouseArea.acceptedButtons: isMenuButton
                                           ? Qt.LeftButton | Qt.RightButton
                                           : Qt.LeftButton

                onClicked: {
                    if (!root.model) {
                        return
                    }

                    if (isMenuButton) {
                        allVoicesMenu.toggleOpened(allVoicesMenu.model)
                    } else {
                        voiceAssignmentSection.propertyItem.value = VoiceTypes.VOICE_ALL_IN_INSTRUMENT
                    }
                }

                FlatButtonMenuIndicatorTriangle {
                    visible: allVoicesButton.isMenuButton
                }

                StyledMenuLoader {
                    id: allVoicesMenu

                    readonly property var model: [
                        {
                            id: "VOICE_ALL_IN_INSTRUMENT",
                            title: qsTrc("inspector", "All voices on instrument"),
                            checkable: true,
                            checked: voiceAssignmentSection.propertyItem?.value === VoiceTypes.VOICE_ALL_IN_INSTRUMENT
                        },
                        {
                            id: "VOICE_ALL_IN_STAFF",
                            title: qsTrc("inspector", "All voices on this staff only"),
                            checkable: true,
                            checked: voiceAssignmentSection.propertyItem?.value === VoiceTypes.VOICE_ALL_IN_STAFF
                        }
                    ]

                    onHandleMenuItem: function (itemId) {
                        if (!voiceAssignmentSection.propertyItem) {
                            return
                        }

                        switch (itemId) {
                        case "VOICE_ALL_IN_INSTRUMENT":
                            voiceAssignmentSection.propertyItem.value = VoiceTypes.VOICE_ALL_IN_INSTRUMENT
                            break
                        case "VOICE_ALL_IN_STAFF":
                            voiceAssignmentSection.propertyItem.value = VoiceTypes.VOICE_ALL_IN_STAFF
                            break
                        }
                    }
                }
            }

            FlatRadioButtonList {
                id: individualVoicesSection

                navigationPanel: root.navigationPanel
                navigationRowStart: allVoicesButton.navigation.row + 1

                Layout.fillWidth: true
                height: 30

                currentValue: voiceAssignmentSection.propertyItem && !voiceAssignmentSection.propertyItem.isUndefined && voiceAssignmentSection.propertyItem.value === VoiceTypes.VOICE_CURRENT_ONLY
                              ? root.model ? root.model.voice.value : undefined
                              : undefined

                model: [
                    { iconCode: IconCode.VOICE_1, value: 0 },
                    { iconCode: IconCode.VOICE_2, value: 1 },
                    { iconCode: IconCode.VOICE_3, value: 2 },
                    { iconCode: IconCode.VOICE_4, value: 3 }
                ]

                onToggled: function(newValue) {
                    if (root.model) {
                        root.model.changeVoice(newValue)
                    }
                }
            }
        }
    }

    FlatRadioButtonGroupPropertyView {
        id: positionSection

        titleText: qsTrc("inspector", "Position")

        propertyItem: root.model ? root.model.voiceBasedPosition : null

        navigationPanel: root.navigationPanel
        navigationRowStart: voiceAssignmentSection.navigationRowEnd + 1

        model: [
            { text: qsTrc("inspector", "Auto"), value: DirectionTypes.VERTICAL_AUTO },
            { text: qsTrc("inspector", "Above"), value: DirectionTypes.VERTICAL_UP },
            { text: qsTrc("inspector", "Below"), value: DirectionTypes.VERTICAL_DOWN }
        ]
    }

    FlatRadioButtonGroupPropertyView {
        id: centerStavesSection

        visible: root.model ? root.model.isMultiStaffInstrument : false
        enabled: root.model ? root.model.isStaveCenteringAvailable : false

        titleText: qsTrc("inspector", "Center between staves")

        propertyItem: root.model ? root.model.centerBetweenStaves : null

        navigationPanel: root.navigationPanel
        navigationRowStart: positionSection.navigationRowEnd + 1

        model: [
            { text: qsTrc("inspector", "Auto"), value: DirectionTypes.CENTER_STAVES_AUTO },
            { text: qsTrc("inspector", "On"), value: DirectionTypes.CENTER_STAVES_ON },
            { text: qsTrc("inspector", "Off"), value: DirectionTypes.CENTER_STAVES_OFF }
        ]
    }
}
