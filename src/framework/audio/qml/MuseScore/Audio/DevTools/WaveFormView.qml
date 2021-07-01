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

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.Audio 1.0

ScrollView {
    id: root

    property real currentSignalAmplitude: 0.0
    property real currentSignalPeak: 0.0
    property int valueScaling: 1000

    property int lineWidth: 2

    function requestPaint() {
        canvas.requestPaint()
    }

    height: 300
    contentWidth: canvas.width
    clip: true

    ScrollBar.horizontal.interactive: true
    ScrollBar.vertical.interactive: true

    Canvas {
        id: canvas

        height: root.height
        width: 10000

        renderStrategy: Canvas.Threaded
        renderTarget: Canvas.FramebufferObject

        onPaint: {
            var ctx = canvas.context

            if (!ctx) {
                ctx = getContext("2d")
            }

            ctx.fillStyle = ui.theme.accentColor
            ctx.fillRect(0, canvasSize.height / 2, lineWidth, -currentSignalAmplitude * valueScaling)
            ctx.fillRect(0, canvasSize.height / 2, lineWidth, currentSignalAmplitude * valueScaling)

            ctx.translate(lineWidth, 0)
        }
    }
}
