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
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.Audio 1.0
import QtQuick.Controls 2.15

Canvas {
    id: root

    property real currentVolumePressure: -60.0
    property real minDisplayedVolumePressure: -60.0
    property real maxDisplayedVolumePressure: 0.0

    property bool showRuler: false

    width: root.showRuler ? prv.indicatorWidth + 20 : prv.indicatorWidth
    height: prv.indicatorHeight + (prv.overloadHeight * 2)

    QtObject {
        id: prv

        property var gradient: null
        property int overloadHeight: 4

        property real indicatorHeight: 140
        property real indicatorWidth: 6

        // value ranges
        property int fullValueRangeLength: Math.abs(root.minDisplayedVolumePressure) + Math.abs(root.maxDisplayedVolumePressure)
        property real divisionPixels: (prv.indicatorHeight - prv.overloadHeight) / fullValueRangeLength

        property real unitsTextWidth: 12
        property color unitTextColor: ui.theme.fontPrimaryColor
        property string unitTextFont: {
            var pxSize = String('8px')
            var family = String('\'' + ui.theme.bodyFont.family + '\'')

            return pxSize + ' ' + family
        }

        // strokes
        property real strokeHorizontalMargin: 2
        property real longStrokeHeight: 1
        property real longStrokeWidth: 5
        property color longStrokeColor: Utils.colorWithAlpha(ui.theme.fontPrimaryColor, 0.5)
        property real shortStrokeHeight: 1
        property real shortStrokeWidth: 2
        property color shortStrokeColor: Utils.colorWithAlpha(ui.theme.fontPrimaryColor, 0.3)
    }

    function drawRuler(ctx, originVPos, originHPos, fullStep, smallStep, strokeHeight, strokeWidth) {
        var currentStrokeVPos = 0

        for (var i = 0; i <= prv.fullValueRangeLength; i+=smallStep) {

            var division = prv.divisionPixels

            if (i == 0) {
                currentStrokeVPos = originVPos
            } else {
                currentStrokeVPos += division * smallStep
            }

            if (i % fullStep) {
                ctx.fillStyle = prv.shortStrokeColor
                ctx.fillRect(currentStrokeVPos,
                             originHPos,
                             prv.shortStrokeHeight,
                             prv.shortStrokeWidth)

            } else {
                ctx.fillStyle = prv.longStrokeColor

                ctx.fillRect(currentStrokeVPos,
                             originHPos,
                             prv.longStrokeHeight,
                             prv.longStrokeWidth)

                ctx.save()

                ctx.rotate(Math.PI/2)
                ctx.fillStyle = prv.unitTextColor
                ctx.fillText(prv.fullValueRangeLength - i, prv.unitsTextWidth + prv.unitsTextWidth/2, -currentStrokeVPos + 2)

                ctx.restore()
            }
        }
    }

    onPaint: {
        var ctx = root.context

        if (!ctx) {
            ctx = getContext("2d")
            ctx.translate(0, root.height)
            ctx.rotate(3 * (Math.PI/2))

            ctx.textAlign = "start"
            ctx.font = prv.unitTextFont

            var originVPos = prv.overloadHeight
            var originHPos = prv.indicatorWidth + prv.strokeHorizontalMargin

            drawRuler(ctx, originVPos, originHPos, 6/*fullStep*/, 3/*smallStep*/)
        }

        ctx.clearRect(0, 0, root.height, prv.indicatorWidth)

        ctx.fillStyle = "#4D4D4D"
        ctx.fillRect(prv.overloadHeight, 0, prv.indicatorHeight, prv.indicatorWidth)

        ctx.fillStyle = "#666666"
        ctx.fillRect(prv.indicatorHeight, 0, prv.overloadHeight, prv.indicatorWidth)

        if (!prv.gradient) {
            prv.gradient = ctx.createLinearGradient(prv.overloadHeight, 0, prv.indicatorHeight - prv.overloadHeight, prv.indicatorWidth)
            prv.gradient.addColorStop(0.0, "#26E386")
            prv.gradient.addColorStop(0.55, "#CBED41")
            prv.gradient.addColorStop(0.80, "#FC8226")
        }

        ctx.fillStyle = prv.gradient
        ctx.fillRect(prv.overloadHeight, 0, prv.divisionPixels * (prv.fullValueRangeLength - Math.abs(root.currentVolumePressure)), prv.indicatorWidth)
    }

    onCurrentVolumePressureChanged: {
        requestPaint()
    }

    Component.onCompleted: {
        requestPaint()
    }
}
