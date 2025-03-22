/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore Limited
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

import Muse.UiComponents 1.0

FlatButton {
    id: root

    signal scrollRequested()

    width: 22
    height: parent.height

    opacity: 1.0

    backgroundItem: Rectangle {
        id: background
        color: ui.theme.backgroundPrimaryColor

        states: [
            State {
                name: "pressed"
                when: root.mouseArea.pressed && root.enabled

                PropertyChanges {
                    target: background
                    color: ui.theme.buttonColor
                    opacity: ui.theme.buttonOpacityHit
                }
            },

            State {
                name: "hovered"
                when: root.mouseArea.containsMouse && root.enabled

                PropertyChanges {
                    target: background
                    color: ui.theme.buttonColor
                    opacity: ui.theme.buttonOpacityHover
                }
            }
        ]

        NavigationFocusBorder {
            navigationCtrl: root.navigation
            drawOutsideParent: false
        }
    }

    mouseArea.preventStealing: true

    mouseArea.onClicked: { root.scrollRequested() }
    mouseArea.onPressAndHold: { timer.running = true }
    mouseArea.onReleased: { timer.running = false }

    onEnabledChanged: {
        // If the button becomes disabled, the mouse area does not emit the
        // `released` signal anymore, so we'll stop the repeat timer here.
        if (!enabled) {
            timer.running = false
        }
    }

    Timer {
        id: timer

        interval: 100
        repeat: true

        onTriggered: { root.scrollRequested() }
    }
}
