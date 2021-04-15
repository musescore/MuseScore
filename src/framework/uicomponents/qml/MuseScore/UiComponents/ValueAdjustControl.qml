/**
 * // SPDX-License-Identifier: GPL-3.0-only
 * // MuseScore-CLA-applies
 * //=============================================================================
 * //  MuseScore
 * //  Music Composition & Notation
 * //
 * //  Copyright (C) 2021 MuseScore BVBA and others
 * //
 * //  This program is free software: you can redistribute it and/or modify
 * //  it under the terms of the GNU General Public License version 3 as
 * //  published by the Free Software Foundation.
 * //
 * //  This program is distributed in the hope that it will be useful,
 * //  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * //  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * //  GNU General Public License for more details.
 * //
 * //  You should have received a copy of the GNU General Public License
 * //  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 * //============================================================================= **/
import QtQuick 2.15
import MuseScore.Ui 1.0

Item {
    id: root

    property var icon

    signal increaseButtonClicked
    signal decreaseButtonClicked

    height: childrenRect.height
    width: childrenRect.width

    Column {
        id: adjustButtonsColumn

        height: childrenRect.height
        width: childrenRect.width

        Loader {
            id: incrementButtonLoader

            height: childrenRect.height
            width: childrenRect.width

            rotation: 180

            sourceComponent: adjustButtonComponent

            MouseArea {
                id: increaseMouseArea

                anchors.fill: parent

                preventStealing: true

                pressAndHoldInterval: 200

                onClicked: {
                    root.increaseButtonClicked()
                }

                onPressAndHold: {
                    continuousIncreaseTimer.running = true
                }

                onReleased: {
                    continuousIncreaseTimer.running = false
                }

                Timer {
                    id: continuousIncreaseTimer

                    interval: 100

                    repeat: true

                    onTriggered: {
                        root.increaseButtonClicked()
                    }
                }
            }
        }

        Loader {
            id: decrementButtonLoader

            height: childrenRect.height
            width: childrenRect.width

            sourceComponent: adjustButtonComponent

            MouseArea {
                id: decreaseMouseArea

                anchors.fill: parent

                preventStealing: true

                pressAndHoldInterval: 200

                onClicked: {
                    root.decreaseButtonClicked()
                }

                onPressAndHold: {
                    continuousDecreaseTimer.running = true
                }

                onReleased: {
                    continuousDecreaseTimer.running = false
                }

                Timer {
                    id: continuousDecreaseTimer

                    interval: 100

                    repeat: true

                    onTriggered: {
                        root.decreaseButtonClicked()
                    }
                }
            }
        }
    }

    Component {
        id: adjustButtonComponent

        Rectangle {
            id: backgroundRect

            color: "transparent"

            width: buttonIcon.width
            height: buttonIcon.height

            StyledIconLabel {
                id: buttonIcon

                iconCode: root.icon
            }
        }
    }
}
