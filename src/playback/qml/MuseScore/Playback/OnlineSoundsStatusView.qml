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
import QtQuick 2.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0

import MuseScore.Playback 1.0

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

    onLoaded: {
        item.model = statusModel
        item.navigation.panel = root.navigationPanel
        item.navigation.order = root.navigationOrder
    }

    sourceComponent: FlatButton {
        id: view

        property var model
        property var navPanel
        property int navOrder: 0

        navigation.name: "OnlineSoundsStatusView"

        readonly property bool mouseAreaEnabled: view.model.canProcessOnlineSounds
                                                 || view.model.status === OnlineSoundsStatusModel.Error

        mouseArea.enabled: view.mouseAreaEnabled
        mouseArea.hoverEnabled: view.mouseAreaEnabled

        // Enable tooltips but disable clicks
        mouseArea.acceptedButtons: view.model.canProcessOnlineSounds ? Qt.LeftButton : Qt.NoButton
        hoverHitColor: view.model.canProcessOnlineSounds ? ui.theme.buttonColor : "transparent"

        transparent: true
        margins: 8

        onClicked: {
            view.model.processOnlineSounds()
        }

        toolTipTitle: view.model.errorTitle
        toolTipDescription: view.model.errorDescription

        contentItem: Row {
            spacing: 6

            Loader {
                width: 16
                height: 16

                sourceComponent: {
                    switch(view.model.status) {
                    case OnlineSoundsStatusModel.Processing: return busyIndicator
                    case OnlineSoundsStatusModel.Error: return errorIndicator
                    case OnlineSoundsStatusModel.Success: {
                        if (view.model.canProcessOnlineSounds) {
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
                    if (view.model.canProcessOnlineSounds) {
                        return qsTrc("playback", "Process online sounds")
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
