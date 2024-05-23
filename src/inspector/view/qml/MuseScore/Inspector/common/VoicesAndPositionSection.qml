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
        id: applyToVoiceSection

        titleText: qsTrc("inspector", "Apply to voice")

        propertyItem: root.model ? root.model.applyToVoice : null

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
                accentButton: applyToVoiceSection.propertyItem
                              && (applyToVoiceSection.propertyItem.value === VoiceTypes.VOICE_ALL_IN_INSTRUMENT
                                  || applyToVoiceSection.propertyItem.value === VoiceTypes.VOICE_ALL_IN_STAFF)
                backgroundRadius: 2 // match FlatRadioButton

                navigation.panel: root.navigationPanel
                navigation.row: applyToVoiceSection.navigationRowStart + 1

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
                        applyToVoiceSection.propertyItem.value = VoiceTypes.VOICE_ALL_IN_INSTRUMENT
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
                            checked: applyToVoiceSection.propertyItem?.value === VoiceTypes.VOICE_ALL_IN_INSTRUMENT
                        },
                        {
                            id: "VOICE_ALL_IN_STAFF",
                            title: qsTrc("inspector", "All voices on this staff only"),
                            checkable: true,
                            checked: applyToVoiceSection.propertyItem?.value === VoiceTypes.VOICE_ALL_IN_STAFF
                        }
                    ]

                    onHandleMenuItem: function (itemId) {
                        if (!applyToVoiceSection.propertyItem) {
                            return
                        }

                        switch (itemId) {
                        case "VOICE_ALL_IN_INSTRUMENT":
                            applyToVoiceSection.propertyItem.value = VoiceTypes.VOICE_ALL_IN_INSTRUMENT
                            break
                        case "VOICE_ALL_IN_STAFF":
                            applyToVoiceSection.propertyItem.value = VoiceTypes.VOICE_ALL_IN_STAFF
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

                currentValue: applyToVoiceSection.propertyItem && !applyToVoiceSection.propertyItem.isUndefined
                              ? applyToVoiceSection.propertyItem.value
                              : undefined

                model: [
                    { iconCode: IconCode.VOICE_1, value: VoiceTypes.VOICE_ONE },
                    { iconCode: IconCode.VOICE_2, value: VoiceTypes.VOICE_TWO },
                    { iconCode: IconCode.VOICE_3, value: VoiceTypes.VOICE_THREE },
                    { iconCode: IconCode.VOICE_4, value: VoiceTypes.VOICE_FOUR }
                ]

                onToggled: function(newValue) {
                    if (applyToVoiceSection.propertyItem) {
                        applyToVoiceSection.propertyItem.value = newValue
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
        navigationRowStart: applyToVoiceSection.navigationRowEnd + 1

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
