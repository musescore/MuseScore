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

import "internal"

ListView {
    id: root

    property bool arrowControlsAvailable: false

    clip: true
    boundsBehavior: Flickable.StopAtBounds
    maximumFlickVelocity: ui.theme.flickableMaxVelocity

    Component.onCompleted: {
        if (!root.arrowControlsAvailable) {
            var scrollBarVerticalObj = scrollBarComp.createObject(root)
            var scrollBarHorizontalObj = scrollBarComp.createObject(root)

            ScrollBar.vertical = scrollBarVerticalObj
            ScrollBar.horizontal = scrollBarHorizontalObj
        }
    }

    Component {
        id: scrollBarComp

        StyledScrollBar {}
    }

    ArrowScrollButton {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        icon: IconCode.UP
        isScrollUp: true
        view: root

        enabled: root.arrowControlsAvailable
    }

    ArrowScrollButton {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right

        icon: IconCode.DOWN
        isScrollUp: false
        view: root

        enabled: root.arrowControlsAvailable
    }
}
