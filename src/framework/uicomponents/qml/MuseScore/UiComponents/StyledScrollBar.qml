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
import QtQuick 2.9
import QtQuick.Controls 2.2

import MuseScore.Ui 1.0

ScrollBar {
    id: root

    property alias color: handle.color
    property alias border: handle.border

    width: orientation === Qt.Vertical ? 10 : 0
    height: orientation === Qt.Horizontal ? 10 : 0

    visible: size !== 0 && size !== 1
    padding: 0

    contentItem: Rectangle {
        id: handle

        radius: 5
        color: ui.theme.fontPrimaryColor
        opacity: root.pressed ? 0.7 : 0.3
        visible: root.active
    }

    function setPosition(position) {
        root.position = position

        if (root.policy === ScrollBar.AlwaysOn) {
            return
        }

        root.active = true

        if (!resetActiveTimer.running) {
            resetActiveTimer.stop()
        }

        resetActiveTimer.start()
    }

    Timer {
        id: resetActiveTimer

        onTriggered: {
            root.active = Qt.binding( function() { return root.hovered || root.pressed } )
        }
    }
}
