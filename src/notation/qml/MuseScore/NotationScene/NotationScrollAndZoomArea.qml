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

import MuseScore.NotationScene 1.0

import "internal"

PinchArea {
    required default property NotationPaintView notationView

    onPinchUpdated: function(pinch) {
        notationView.pinchToZoom(pinch.scale / pinch.previousScale, pinch.center)
    }

    // A macOS feature which allows double-tapping with two fingers to zoom in or out
    onSmartZoom: function(pinch) {
        notationView.pinchToZoom(pinch.scale === 0 ? 0.5 : 2, pinch.center)
    }

    children: [ notationView, horizontalScrollBar, verticalScrollBar ]

    NotationScrollBar {
        id: horizontalScrollBar
        orientation: Qt.Horizontal

        size: notationView.horizontalScrollbarSize
        position: notationView.startHorizontalScrollPosition

        onMoved: function(newPosition) {
            notationView.scrollHorizontal(newPosition)
        }
    }

    NotationScrollBar {
        id: verticalScrollBar
        orientation: Qt.Vertical

        size: notationView.verticalScrollbarSize
        position: notationView.startVerticalScrollPosition

        onMoved: function(newPosition) {
            notationView.scrollVertical(newPosition)
        }
    }
}
