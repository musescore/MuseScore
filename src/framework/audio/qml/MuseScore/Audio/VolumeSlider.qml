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

Slider {
    id: root

    height: 140 + prv.handleHeight
    width: 32 + prv.unitsTextWidth

    from: -48
    to: 12
    value: 0
    stepSize: 0.1
    orientation: Qt.Vertical

    QtObject {
        id: prv

        property real rulerLineWidth: 2
        property real rulerLineHeight: root.height - prv.handleHeight

        // value ranges
        property real lowAccuracyEdge: -12
        property real lowAccuracyDivisionPixels: (prv.rulerLineHeight / 2) / prv.lowAccuracyRange
        property real lowAccuracyRange: Math.abs(root.from) - Math.abs(prv.lowAccuracyEdge)

        property real highAccuracyRange: Math.abs(prv.lowAccuracyEdge) + Math.abs(root.to)
        property real highAccuracyDivisionPixels: (prv.rulerLineHeight / 2) / highAccuracyRange

        property int fullValueRangeLength: Math.abs(root.from) + Math.abs(root.to)

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
        property real longStrokeWidth: 8
        property color longStrokeColor: Utils.colorWithAlpha(ui.theme.fontPrimaryColor, 0.5)
        property real shortStrokeHeight: 1
        property real shortStrokeWidth: 4
        property color shortStrokeColor: Utils.colorWithAlpha(ui.theme.fontPrimaryColor, 0.3)

        property real handleWidth: 16
        property real handleHeight: 32
    }

    background: Canvas {
        id: bgCanvas

        height: root.height
        width: root.width

        function drawRuler(ctx, originVPos, originHPos, fullStep, smallStep, strokeHeight, strokeWidth) {
            var currentStrokeVPos = 0

            for (var i = 0; i <= prv.fullValueRangeLength; i+=smallStep) {

                var division = 0
                if (i <= prv.lowAccuracyRange) {
                    division = prv.lowAccuracyDivisionPixels
                } else {
                    division = prv.highAccuracyDivisionPixels
                }

                if (i == 0) {
                    currentStrokeVPos = originVPos
                } else {
                    currentStrokeVPos += division * smallStep
                }

                if (i % fullStep) {
                    ctx.fillStyle = prv.shortStrokeColor
                    ctx.fillRect(currentStrokeVPos,
                                 originHPos - prv.shortStrokeWidth,
                                 prv.shortStrokeHeight,
                                 prv.shortStrokeWidth)

                } else {
                    ctx.fillStyle = prv.longStrokeColor

                    ctx.fillRect(currentStrokeVPos,
                                 originHPos - prv.longStrokeWidth,
                                 prv.longStrokeHeight,
                                 prv.longStrokeWidth)

                    ctx.save()

                    ctx.rotate(Math.PI/2)
                    ctx.fillStyle = prv.unitTextColor
                    ctx.fillText(textByDbValue(root.from + i), prv.unitsTextWidth + prv.unitsTextWidth/2, -currentStrokeVPos + 2)

                    ctx.restore()
                }
            }
        }

        function textByDbValue(dbValue) {
            if (dbValue === root.from) {
                return "-" + "\u221E" // minus infinity unicode
            } else {
                return String(dbValue)
            }
        }

        onPaint: {
            var ctx = root.context

            if (!ctx) {
                ctx = getContext("2d")
                ctx.translate(0, height)
                ctx.rotate(3 * (Math.PI/2))

                ctx.textAlign = "end"
                ctx.font = prv.unitTextFont
            }

            ctx.fillStyle = Utils.colorWithAlpha(ui.theme.fontPrimaryColor, 0.5)

            ctx.fillRect(prv.handleHeight/2, // vertical pos
                         bgCanvas.width - prv.handleWidth/2 - prv.rulerLineWidth/2, // horizontal pos
                         prv.rulerLineHeight,
                         prv.rulerLineWidth)

            var originVPos = prv.handleHeight / 2
            var originHPos = bgCanvas.width - prv.handleWidth/2 - prv.rulerLineWidth - prv.strokeHorizontalMargin - 2

            drawRuler(ctx, originVPos, originHPos, 6/*fullStep*/, 2/*smallStep*/)
        }
    }

    handle: Rectangle {
        id: handleItem

        x: root.width - prv.handleWidth
        y: (1 - root.position) * prv.rulerLineHeight
        implicitWidth: prv.handleWidth
        implicitHeight: prv.handleHeight
        radius: 2
        color: ui.theme.fontPrimaryColor
        border.color: "#00000075"

        Rectangle {
            anchors {
                verticalCenter: parent.verticalCenter
                left: parent.left
                right: parent.right
                leftMargin: 2
                rightMargin: 2
            }

            height: 1
            radius: prv.handleHeight / 2
            color: "#000000"
        }
    }

    Component.onCompleted: {
        bgCanvas.requestPaint()
    }
}
