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
import QtQuick

import "internal"

PinchArea {
    id: root

    required default property Item view

    property alias horizontalScrollbarSize: horizontalScrollBar.size
    property alias startHorizontalScrollPosition: horizontalScrollBar.position

    property alias verticalScrollbarSize: verticalScrollBar.size
    property alias startVerticalScrollPosition: verticalScrollBar.position

    property real scrollbarThickness: 10
    property bool scrollbarAlwaysVisibleWhenNeeded: false
    property real scrollbarOpacityNormal: 0.3
    property real scrollbarOpacityPressed: 0.7

    signal pinchToZoom(real scale, var pos)
    signal scrollHorizontal(real newPos)
    signal scrollVertical(real newPos)

    onPinchUpdated: function(pinch) {
        root.pinchToZoom(pinch.scale / pinch.previousScale, pinch.center)
    }

    // A macOS feature which allows double-tapping with two fingers to zoom in or out
    onSmartZoom: function(pinch) {
        root.pinchToZoom(pinch.scale === 0 ? 0.5 : 2, pinch.center)
    }

    children: [ view, horizontalScrollBar, verticalScrollBar ]

    ViewScrollBar {
        id: horizontalScrollBar
        orientation: Qt.Horizontal

        thickness: root.scrollbarThickness
        alwaysVisibleWhenNeeded: root.scrollbarAlwaysVisibleWhenNeeded
        opacityNormal: root.scrollbarOpacityNormal
        opacityPressed: root.scrollbarOpacityPressed

        onMoved: function(newPosition) {
            root.scrollHorizontal(newPosition)
        }
    }

    ViewScrollBar {
        id: verticalScrollBar
        orientation: Qt.Vertical

        thickness: root.scrollbarThickness
        alwaysVisibleWhenNeeded: root.scrollbarAlwaysVisibleWhenNeeded
        opacityNormal: root.scrollbarOpacityNormal
        opacityPressed: root.scrollbarOpacityPressed

        onMoved: function(newPosition) {
            root.scrollVertical(newPosition)
        }
    }
}
