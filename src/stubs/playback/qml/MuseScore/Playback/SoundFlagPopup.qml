/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore Limited
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

StyledPopupView {
    id: root

    property NavigationSection notationViewNavigationSection
    property int navigationOrderStart
    property int navigationOrderEnd

    contentWidth: content.width
    contentHeight: content.height

    function updatePosition(elementRect) {
        var h = Math.max(root.contentHeight, 360)
        root.x = elementRect.x + elementRect.width + 12
        root.y = elementRect.y - h / 2
    }

    Rectangle {
        id: content

        width: 300
        height: stub.implicitHeight

        color: ui.theme.backgroundSecondaryColor

        StyledTextLabel {
            id: stub
            anchors.centerIn: parent
            text: "Sound Flags Stub"
        }
    }
}
