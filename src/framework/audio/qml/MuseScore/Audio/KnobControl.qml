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

Dial {
    id: root

    property int valueScale: 100

    width: prv.radius * 2
    height: width

    from: -root.valueScale
    to: root.valueScale
    value: 0

    QtObject {
        id: prv

        readonly property real radius: 16
        readonly property bool reversed: root.angle < 0

        readonly property real handlerHeight: 8
        readonly property real handlerWidth: 2

        readonly property real outerArcLineWidth: 3
        readonly property real innerArcLineWidth: 2

        readonly property color valueArcColor: ui.theme.accentColor
        readonly property color outerArcColor: Utils.colorWithAlpha(ui.theme.buttonColor, 0.7)
        readonly property color innerArcColor: Utils.colorWithAlpha(ui.theme.fontPrimaryColor, 0.5)

        onValueArcColorChanged: { backgroundCanvas.requestPaint() }
        onOuterArcColorChanged: { backgroundCanvas.requestPaint() }
        onInnerArcColorChanged: { backgroundCanvas.requestPaint() }
    }

    background: Canvas {
        id: backgroundCanvas

        width: prv.radius * 2
        height: width

        antialiasing: true

        onPaint: {
            var ctx = backgroundCanvas.context

            if (!ctx) {
                ctx = getContext("2d")
                ctx.lineCap = "squared"
            }

            ctx.clearRect(0, 0, canvasSize.width, canvasSize.height)
            ctx.lineWidth = prv.outerArcLineWidth

            ctx.strokeStyle = prv.outerArcColor
            ctx.beginPath()
            ctx.arc(width/2, height/2, prv.radius - prv.outerArcLineWidth, -140 * (Math.PI/180) - Math.PI/2, 140 * (Math.PI/180) - Math.PI/2, false)
            ctx.stroke()

            ctx.strokeStyle = prv.valueArcColor
            ctx.beginPath()
            ctx.arc(width/2, height/2, prv.radius - prv.outerArcLineWidth, -Math.PI/2, root.angle * (Math.PI/180) - Math.PI/2, prv.reversed)
            ctx.stroke()

            ctx.lineWidth = prv.innerArcLineWidth
            ctx.strokeStyle = prv.innerArcColor
            ctx.beginPath()
            ctx.arc(width/2, height/2, prv.radius - (prv.outerArcLineWidth + prv.innerArcLineWidth), 0, Math.PI * 2, false)
            ctx.stroke()
        }
    }

    handle: Rectangle {
        x: prv.radius - prv.handlerWidth / 2
        y: prv.outerArcLineWidth + prv.innerArcLineWidth + 2

        height: prv.handlerHeight
        width: prv.handlerWidth
        radius: prv.handlerWidth / 2

        color: ui.theme.fontPrimaryColor
        antialiasing: true

        transformOrigin: Item.Bottom
        rotation: root.angle
    }

    onValueChanged: {
        backgroundCanvas.requestPaint()
    }

    Component.onCompleted: {
        backgroundCanvas.requestPaint()
    }
}
