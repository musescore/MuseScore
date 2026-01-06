/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

pragma ComponentBehavior: Bound

import QtQuick

import Muse.Ui
import Muse.UiComponents

import MuseScore.Playback

Loader {
    id: root

    property var navigationPanel
    property int navigationOrder: 0

    active: statusModel.hasOnlineSounds

    OnlineSoundsStatusModel {
        id: statusModel
    }

    Component.onCompleted: {
        statusModel.load()
    }

    sourceComponent: FlatButton {
        id: view

        navigation.name: "OnlineSoundsStatusView"
        navigation.panel: root.navigationPanel
        navigation.order: root.navigationOrder

        readonly property bool mouseAreaEnabled: statusModel.manualProcessingAllowed
                                                 || statusModel.status === OnlineSoundsStatusModel.Error

        mouseArea.enabled: view.mouseAreaEnabled
        mouseArea.hoverEnabled: view.mouseAreaEnabled

        // Enable tooltips but disable clicks
        mouseArea.acceptedButtons: statusModel.manualProcessingAllowed ? Qt.LeftButton : Qt.NoButton
        hoverHitColor: statusModel.manualProcessingAllowed ? ui.theme.buttonColor : "transparent"

        transparent: true
        margins: 8

        onClicked: {
            statusModel.processOnlineSounds()
        }

        toolTipTitle: statusModel.errorTitle
        toolTipDescription: statusModel.errorDescription

        contentItem: Row {
            spacing: 6

            Loader {
                width: 16
                height: 16

                sourceComponent: {
                    switch (statusModel.status) {
                    case OnlineSoundsStatusModel.Processing:
                        return busyIndicator
                    case OnlineSoundsStatusModel.Error:
                        return errorIndicator
                    case OnlineSoundsStatusModel.Success: {
                        if (statusModel.manualProcessingAllowed) {
                            return processOnlineSoundsIndicator
                        }

                        return successIndicator
                    }
                    }

                    return undefined
                }
            }

            StyledTextLabel {
                anchors.verticalCenter: parent.verticalCenter

                horizontalAlignment: Text.AlignLeft

                text: {
                    switch (statusModel.status) {
                    case OnlineSoundsStatusModel.Processing:
                        return qsTrc("playback", "Processing online sounds")
                    case OnlineSoundsStatusModel.Error:
                        return qsTrc("playback", "Online sounds")
                    case OnlineSoundsStatusModel.Success: {
                        if (statusModel.manualProcessingAllowed) {
                            return qsTrc("playback", "Process online sounds")
                        }

                        return qsTrc("playback", "Online sounds processed")
                    }
                    }

                    return qsTrc("playback", "Online sounds")
                }
            }
        }

        Component {
            id: successIndicator

            Rectangle {
                anchors.fill: parent

                radius: width / 2
                color: "#46A955"

                StyledIconLabel {
                    anchors.centerIn: parent

                    iconCode: IconCode.TICK_RIGHT_ANGLE_THICK
                    color: "white"
                }
            }
        }

        Component {
            id: errorIndicator

            StyledIconLabel {
                anchors.fill: parent

                iconCode: IconCode.WARNING_SMALL
            }
        }

        Component {
            id: busyIndicator

            StyledBusyIndicator {
                backgroundColor: ui.theme.buttonColor
            }
        }

        Component {
            id: processOnlineSoundsIndicator

            StyledIconLabel {
                anchors.fill: parent

                iconCode: IconCode.UPDATE
            }
        }
    }
}
