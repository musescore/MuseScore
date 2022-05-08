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
    property int scrollBarThickness: 8
    property int scrollBarPolicy: ScrollBar.AsNeeded

    clip: true
    boundsBehavior: Flickable.StopAtBounds
    maximumFlickVelocity: ui.theme.flickableMaxVelocity

    ScrollBar.vertical: root.arrowControlsAvailable ? null : scrollBarComp.createObject(root)
    ScrollBar.horizontal: root.arrowControlsAvailable ? null : scrollBarComp.createObject(root)

    Component {
        id: scrollBarComp

        StyledScrollBar {
            thickness: root.scrollBarThickness
            policy: root.scrollBarPolicy
        }
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
