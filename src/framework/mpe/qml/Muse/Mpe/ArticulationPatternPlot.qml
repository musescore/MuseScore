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
import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.Mpe 1.0

Item {
    id: root

    property QtObject patternModel: null

    property bool thumbnailModeOn: false

    property bool showArrangement: false
    property bool showPitch: false
    property bool showExpression: false

    property real zoomLevel: 1.0
    property real minZoomLevel: 0.25
    property real maxZoomLevel: 2.5

    property color arrangementLineColor: "#2093FE"
    property color pitchLineColor: "#27A341"
    property color expressionLineColor: "#F25555"

    property real durationFactor: root.patternModel ? root.patternModel.durationFactor : 1
    property real timestampShiftFactor: root.patternModel ? root.patternModel.timestampShiftFactor : 0

    property int selectedPitchOffsetIndex: root.patternModel ? root.patternModel.selectedPitchOffsetIndex : -1
    property var pitchOffsets: root.patternModel ? root.patternModel.pitchOffsets : []

    property int selectedDynamicOffsetIndex: root.patternModel ? root.patternModel.selectedDynamicOffsetIndex : -1
    property var dynamicOffsets: root.patternModel ? root.patternModel.dynamicOffsets : []

    width: 1000
    height: 1000

    QtObject {
        id: prv

        readonly property int displayableSteps: root.zoomLevel * 24 // from -5 to 15, where 10 is 100% of duration
        readonly property real pixelsPerStep: Math.max(root.width, root.height) / prv.displayableSteps
        readonly property real valuePerStep: root.patternModel ? root.patternModel.singlePercentValue : 1
        readonly property int stepsToFullScale: 10 // amount of steps from 0% to 100% of duration
        readonly property real pixelsToFullScale: prv.stepsToFullScale * prv.pixelsPerStep // amount of steps from 0% to 100% of duration in pixels

        readonly property real pointHandleDiameter: 8

        property real centerXRatio: 0.25
        property real centerYRatio: 0.5

        readonly property real centerX: root.width * prv.centerXRatio
        readonly property real centerY: root.height * prv.centerYRatio

        readonly property real defaultFontSize: 10
        readonly property real fontSize: (1 / root.zoomLevel) * prv.defaultFontSize
        readonly property string fontFamily: "sans-serif"
        readonly property real legendX: root.width * 0.05
        readonly property real legendY: root.height * 0.05
        readonly property real legendSampleSize: 12

        function coordinateValueToSteps(value) {
            return value / prv.valuePerStep
        }

        function stepsToCoordinateValue(steps) {
            return steps * prv.valuePerStep
        }

        function applyXShiftToCoordinates(curvePoint, xShift, xSpanFactor) {
            return Qt.point(curvePoint.x *
                            coordinateValueToSteps(xSpanFactor) / 100 +
                            xShift, curvePoint.y)
        }

        function coordinatesToPixels(curvePoint) {
            return Qt.point(prv.centerX + (prv.coordinateValueToSteps(curvePoint.x) / 100) * prv.pixelsToFullScale,
                            prv.centerY - (prv.coordinateValueToSteps(curvePoint.y) / 100) * prv.pixelsToFullScale)
        }

        function coordinatesFromPixels(pixelsPoint) {
            return Qt.point(prv.stepsToCoordinateValue((pixelsPoint.x - prv.centerX) / prv.pixelsToFullScale),
                            (prv.stepsToCoordinateValue(prv.centerY - pixelsPoint.y) / prv.pixelsToFullScale))
        }

        function legendsList() {
            var result = []

            if (root.showArrangement) {
                result.push({ title : /*qsTrc*/ "Time line", color: root.arrangementLineColor})
            }

            if (root.showPitch) {
                result.push({ title : /*qsTrc*/ "Pitch curve", color: root.pitchLineColor})
            }

            if (root.showExpression) {
                result.push({ title : /*qsTrc*/ "Expression curve", color: root.expressionLineColor})
            }

            return result
        }
    }

    Canvas {
        id: canvas

        anchors.fill: parent

        onPaint: {
            var ctx = canvas.context

            if (!ctx) {
                ctx = getContext("2d")
            }

            canvas.fillBackground(ctx, "#4D4D4D")

            canvas.drawGrid(ctx, 1, "#5D5D5D")

            canvas.drawAxisX(ctx, 2, "#FFFFFF")
            canvas.drawAxisY(ctx, 2, "#FFFFFF")

            if (!root.patternModel) {
                return
            }

            if (!root.thumbnailModeOn) {
                canvas.drawAvailableLegends(ctx, "#FFFFFF")
            }

            if (root.showArrangement) {
                canvas.drawArrangement(ctx)
            }

            if (root.showPitch) {
                canvas.drawPitchCurve(ctx)
            }

            if (root.showExpression) {
                canvas.drawExpressionCurve(ctx)
            }
        }

        function fillBackground(ctx, color) {
            ctx.fillStyle = color
            ctx.fillRect(0, 0, canvas.width, canvas.height)
        }

        function drawGrid(ctx, lineWidth, color) {
            ctx.fillStyle = color

            for (var i = 0; i < prv.displayableSteps; ++i) {
                var idx = i * prv.pixelsPerStep - lineWidth / 2
                ctx.fillRect(idx, 0, lineWidth, canvas.height)
                ctx.fillRect(0, idx, canvas.width, lineWidth)
            }
        }

        function drawAxisX(ctx, lineWidth, color) {
            ctx.fillStyle = color
            ctx.font = prv.fontSize + "px" + " " + prv.fontFamily
            ctx.fillRect(0, (canvas.height * prv.centerYRatio) - (lineWidth / 2), canvas.width, lineWidth)

            if (root.thumbnailModeOn) {
                return
            }

            for (var i = -prv.stepsToFullScale / 2; i <= prv.stepsToFullScale; ++i) {
                var textNum = i * 10 + "%"
                ctx.fillText(textNum,
                             prv.centerX + (i * prv.pixelsPerStep) - (1 / root.zoomLevel) * 8,
                             prv.centerY + (1 / root.zoomLevel) * 24)
            }
        }

        function drawAxisY(ctx, lineWidth, color) {
            ctx.fillStyle = color
            ctx.fillRect((canvas.width * prv.centerXRatio) - (lineWidth / 2), 0, lineWidth, canvas.height)
        }

        function drawAvailableLegends(ctx, textColor) {
            var legends = prv.legendsList()

            for (var i = 0; i < legends.length; ++i) {
                drawLegend(ctx, prv.legendX, prv.legendY + (i * prv.legendSampleSize) + 12, legends[i].color, textColor, legends[i].title)
            }
        }

        function drawLegend(ctx, x, y, sampleColor, textColor, text) {
            if (root.thumbnailModeOn) {
                return
            }

            ctx.fillStyle = sampleColor
            ctx.font = prv.defaultFontSize + "px" + " " + prv.fontFamily
            ctx.fillRect(x, y, prv.legendSampleSize, prv.legendSampleSize)

            ctx.fillStyle = textColor
            ctx.fillText("- " + text, x + prv.legendSampleSize + 4, y + (prv.legendSampleSize / 2) + 4)
        }

        function drawCurve(ctx, points, posXShift, spanFactor, lineWidth, lineColor) {
            ctx.strokeStyle = lineColor
            ctx.lineWidth = lineWidth
            ctx.lineJoin = "round"

            ctx.beginPath()

            for (var i = 0; i < points.length; ++i) {
                var position = prv.coordinatesToPixels(prv.applyXShiftToCoordinates(points[i], posXShift, spanFactor))

                if (i == 0) {
                    ctx.moveTo(position.x, position.y)
                } else {
                    ctx.lineTo(position.x, position.y)
                }
            }

            ctx.stroke()
        }

        function drawPointHandlers(ctx, points, posXShift, spanFactor, selectedPointIdx, handlePrimaryFillColor, handleSecondaryFillColor) {
            if (root.thumbnailModeOn) {
                return
            }

            for (var i = 0; i < points.length; ++i) {
                var position = prv.coordinatesToPixels(prv.applyXShiftToCoordinates(points[i], posXShift, spanFactor))

                var outerDiameter = prv.pointHandleDiameter
                var innerDiameter = prv.pointHandleDiameter / 2

                if (i === selectedPointIdx) {
                    outerDiameter *= 2
                    innerDiameter *= 2
                }

                canvas.drawCircle(ctx, position, handlePrimaryFillColor, outerDiameter)
                canvas.drawCircle(ctx, position, handleSecondaryFillColor, innerDiameter)
            }
        }

        function drawCircle(ctx, position, fillColor, diameter) {
            ctx.beginPath()
            ctx.ellipse(position.x - (diameter / 2),
                        position.y - (diameter / 2),
                        diameter,
                        diameter)

            ctx.fillStyle = fillColor
            ctx.fill()
        }

        function drawArrangement(ctx) {
            var arrangementPoints = [ Qt.point(root.timestampShiftFactor, 0), Qt.point(root.timestampShiftFactor + root.durationFactor, 0)]

            canvas.drawCurve(ctx, arrangementPoints, 0, prv.valuePerStep * 100, 2, root.arrangementLineColor)
            canvas.drawPointHandlers(ctx, arrangementPoints, 0, prv.valuePerStep * 100, 0, "#FFFFFF", root.arrangementLineColor)
        }

        function drawPitchCurve(ctx) {
            canvas.drawCurve(ctx, root.pitchOffsets, root.timestampShiftFactor, root.durationFactor, 2, root.pitchLineColor)
            canvas.drawPointHandlers(ctx, root.pitchOffsets, root.timestampShiftFactor, root.durationFactor, root.selectedPitchOffsetIndex, "#FFFFFF", root.pitchLineColor)
        }

        function drawExpressionCurve(ctx) {
            canvas.drawCurve(ctx, root.dynamicOffsets, root.timestampShiftFactor, root.durationFactor, 2, root.expressionLineColor)
            canvas.drawPointHandlers(ctx, root.dynamicOffsets, root.timestampShiftFactor, root.durationFactor, root.selectedDynamicOffsetIndex, "#FFFFFF", root.expressionLineColor)
        }
    }

    Loader {
        anchors.fill: parent
        active: !root.thumbnailModeOn
        sourceComponent: MouseArea {
            id: plotMouseArea

            property real lastX: 0

            acceptedButtons: Qt.MiddleButton
            scrollGestureEnabled: true

            onPressAndHold: function(mouse) {
                plotMouseArea.lastX = mouse.x
            }

            onPositionChanged: function(mouse) {
                if (mouse.wasHeld) {
                    var shift = (mouse.x - plotMouseArea.lastX) /  root.width
                    prv.centerXRatio += shift
                    canvas.requestPaint()
                    plotMouseArea.lastX = mouse.x
                }
            }

            onWheel: function(wheel) {
                var shift = wheel.angleDelta.y / 360
                var newZoomLevel = root.zoomLevel -= shift

                if (newZoomLevel < root.minZoomLevel) {
                    root.zoomLevel = root.minZoomLevel
                } else if (newZoomLevel > root.maxZoomLevel) {
                    root.zoomLevel = root.maxZoomLevel
                } else {
                    root.zoomLevel = newZoomLevel
                }
            }

            onReleased: {
                plotMouseArea.lastX = 0
            }
        }
    }

    onZoomLevelChanged: {
        canvas.requestPaint()
    }

    onShowArrangementChanged: {
        canvas.requestPaint()
    }

    onShowPitchChanged: {
        canvas.requestPaint()
    }

    onShowExpressionChanged: {
        canvas.requestPaint()
    }

    onDurationFactorChanged: {
        if (root.showArrangement) {
            canvas.requestPaint()
        }
    }

    onTimestampShiftFactorChanged: {
        if (root.showArrangement) {
            canvas.requestPaint()
        }
    }

    onPitchOffsetsChanged: {
        if (root.showPitch) {
            canvas.requestPaint()
        }
    }

    onSelectedPitchOffsetIndexChanged: {
        if (root.showPitch) {
            canvas.requestPaint()
        }
    }

    onDynamicOffsetsChanged: {
        if (root.showExpression) {
            canvas.requestPaint()
        }
    }

    onSelectedDynamicOffsetIndexChanged: {
        if (root.showExpression) {
            canvas.requestPaint()
        }
    }
}
