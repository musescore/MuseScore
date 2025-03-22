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

import Muse.Ui 1.0
import Muse.UiComponents 1.0

Column {
    id: root

    readonly property bool containsMouse: (increaseMouseArea.containsMouse && increaseButton.enabled)
                                          || (decreaseMouseArea.containsMouse && decreaseButton.enabled)

    property alias canIncrease: increaseButton.enabled
    property alias canDecrease: decreaseButton.enabled

    signal increaseButtonClicked
    signal decreaseButtonClicked

    width: 22
    height: parent.height

    property real radius: 2

    RoundedRectangle {
        id: increaseButton
        width: parent.width
        height: parent.height / 2

        color: "transparent"
        topRightRadius: root.radius

        StyledIconLabel {
            anchors.centerIn: parent
            iconCode: IconCode.SMALL_ARROW_UP
        }

        onEnabledChanged: {
            // If the button becomes disabled, the mouse area does not emit the
            // `released` signal anymore, so we'll stop the repeat timer here.
            if (!enabled) {
                continuousIncreaseTimer.running = false
            }
        }

        MouseArea {
            id: increaseMouseArea
            anchors.fill: parent

            hoverEnabled: true
            preventStealing: true

            onClicked: { root.increaseButtonClicked() }
            onPressAndHold: { continuousIncreaseTimer.running = true }
            onReleased: { continuousIncreaseTimer.running = false }

            Timer {
                id: continuousIncreaseTimer

                interval: 100
                repeat: true

                onTriggered: { root.increaseButtonClicked() }
            }
        }

        states: [
            State {
                name: "hovered"
                when: increaseMouseArea.containsMouse && !increaseMouseArea.pressed && increaseButton.enabled

                PropertyChanges {
                    target: increaseButton
                    color: Utils.colorWithAlpha(ui.theme.buttonColor, ui.theme.buttonOpacityHover)
                }
            },

            State {
                name: "pressed"
                when: increaseMouseArea.pressed && increaseButton.enabled

                PropertyChanges {
                    target: increaseButton
                    color: Utils.colorWithAlpha(ui.theme.buttonColor, ui.theme.buttonOpacityHit)
                }
            }
        ]
    }

    RoundedRectangle {
        id: decreaseButton
        width: parent.width
        height: parent.height / 2

        color: "transparent"
        bottomRightRadius: root.radius

        StyledIconLabel {
            anchors.centerIn: parent
            iconCode: IconCode.SMALL_ARROW_DOWN
        }

        onEnabledChanged: {
            // If the button becomes disabled, the mouse area does not emit the
            // `released` signal anymore, so we'll stop the repeat timer here.
            if (!enabled) {
                continuousDecreaseTimer.running = false
            }
        }

        MouseArea {
            id: decreaseMouseArea
            anchors.fill: parent

            hoverEnabled: true
            preventStealing: true

            onClicked: { root.decreaseButtonClicked() }
            onPressAndHold: { continuousDecreaseTimer.running = true }
            onReleased: { continuousDecreaseTimer.running = false }

            Timer {
                id: continuousDecreaseTimer

                interval: 100
                repeat: true

                onTriggered: { root.decreaseButtonClicked() }
            }
        }

        states: [
            State {
                name: "hovered"
                when: decreaseMouseArea.containsMouse && !decreaseMouseArea.pressed && decreaseButton.enabled

                PropertyChanges {
                    target: decreaseButton
                    color: Utils.colorWithAlpha(ui.theme.buttonColor, ui.theme.buttonOpacityHover)
                }
            },

            State {
                name: "pressed"
                when: decreaseMouseArea.pressed && decreaseButton.enabled

                PropertyChanges {
                    target: decreaseButton
                    color: Utils.colorWithAlpha(ui.theme.buttonColor, ui.theme.buttonOpacityHit)
                }
            }
        ]
    }
}
