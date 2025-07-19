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

import Muse.Ui 1.0
import Muse.UiComponents 1.0

Slider {
    id: root

    property real volumeLevel: 0.0
    property real readableVolumeLevel: Math.round(root.volumeLevel * 10) / 10

    property alias navigation: navCtrl

    signal volumeLevelMoved(var level)

    height: 140 + prv.handleHeight
    width: 32 + prv.unitsTextWidth

    from: -60
    to: 12
    value: convertor.volumeLevelToLocal(root.volumeLevel)
    stepSize: 0.1
    orientation: Qt.Vertical
    wheelEnabled: false

    signal increaseRequested()
    signal decreaseRequested()

    QtObject {
        id: convertor

        readonly property real highAccuracyStep: 1.5
        readonly property real lowAccuracyStep: 0.75

        readonly property real localCenter: -24
        readonly property real logicalCenter: -12

        function volumeLevelToLocal(newValue) {
            var diff

            if (newValue > convertor.logicalCenter) {
                diff = root.to - newValue
                return root.to - (diff * convertor.highAccuracyStep)
            } else {
                diff = convertor.logicalCenter - newValue
                return convertor.localCenter - (diff * convertor.lowAccuracyStep)
            }
        }

        function volumeLevelFromLocal(newValue) {
            var diff

            if (newValue > convertor.localCenter) {
                diff = root.to - newValue
                return root.to - (diff / convertor.highAccuracyStep)
            } else {
                diff = convertor.localCenter - newValue
                return convertor.logicalCenter - (diff / convertor.lowAccuracyStep)
            }
        }
    }

    QtObject {
        id: prv

        readonly property real rulerLineWidth: 2
        readonly property real rulerLineHeight: root.height - prv.handleHeight

        // value ranges
        readonly property real lowAccuracyEdge: -12
        readonly property real lowAccuracyDivisionPixels: (prv.rulerLineHeight / 2) / prv.lowAccuracyRange
        readonly property real lowAccuracyRange: Math.abs(root.from) - Math.abs(prv.lowAccuracyEdge)

        readonly property real highAccuracyRange: Math.abs(prv.lowAccuracyEdge) + Math.abs(root.to)
        readonly property real highAccuracyDivisionPixels: (prv.rulerLineHeight / 2) / highAccuracyRange

        readonly property int fullValueRangeLength: Math.abs(root.from) + Math.abs(root.to)

        readonly property real unitsTextWidth: 12
        readonly property color unitTextColor: Utils.colorWithAlpha(ui.theme.fontPrimaryColor, 0.8)
        readonly property string unitTextFont: {
            var pxSize = String('8px')
            var family = String('\'' + ui.theme.bodyFont.family + '\'')

            return pxSize + ' ' + family
        }

        onUnitTextColorChanged: { bgCanvas.requestPaint() }
        onUnitTextFontChanged: { bgCanvas.requestPaint() }

        // strokes
        readonly property real strokeHorizontalMargin: 2
        readonly property real longStrokeHeight: 1
        readonly property real longStrokeWidth: 8
        readonly property color longStrokeColor: Utils.colorWithAlpha(ui.theme.fontPrimaryColor, 0.5)
        readonly property real shortStrokeHeight: 1
        readonly property real shortStrokeWidth: 4
        readonly property color shortStrokeColor: Utils.colorWithAlpha(ui.theme.fontPrimaryColor, 0.3)

        onLongStrokeColorChanged: { bgCanvas.requestPaint() }
        onShortStrokeColorChanged: { bgCanvas.requestPaint() }

        readonly property real handleWidth: 16
        readonly property real handleHeight: 32

        property real dragStartOffset: 0.0
    }

    NavigationControl {
        id: navCtrl
        name: root.objectName != "" ? root.objectName : "VolumeSlider"
        enabled: root.enabled && root.visible

        accessible.role: MUAccessible.Range
        accessible.visualItem: root

        accessible.value: root.readableVolumeLevel
        accessible.minimumValue: root.from
        accessible.maximumValue: root.to
        accessible.stepSize: root.stepSize

        onNavigationEvent: function(event) {
            switch(event.type) {
            case NavigationEvent.Left:
                root.decreaseRequested()
                event.accepted = true
                break
            case NavigationEvent.Right:
                root.increaseRequested()
                event.accepted = true
                break
            }
        }
    }

    background: Canvas {
        id: bgCanvas

        height: root.height
        width: root.width

        NavigationFocusBorder {
            navigationCtrl: navCtrl
        }

        function drawRuler(ctx, originVPos, originHPos, fullStep, smallStep, strokeHeight, strokeWidth) {
            var currentStrokeVPos = 0

            for (var i = 0; i <= prv.fullValueRangeLength; i+=smallStep) {

                var division = 0
                if (i <= prv.lowAccuracyRange) {
                    division = prv.lowAccuracyDivisionPixels
                } else {
                    division = prv.highAccuracyDivisionPixels
                }

                if (i === 0) {
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

                    let textHPos = originHPos - prv.longStrokeWidth - prv.strokeHorizontalMargin

                    ctx.save()

                    ctx.rotate(Math.PI/2)
                    ctx.fillStyle = prv.unitTextColor
                    ctx.fillText(textByDbValue(root.from + i), textHPos, -currentStrokeVPos + 2)

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
            var ctx = bgCanvas.context

            if (!ctx) {
                ctx = getContext("2d")

                ctx.translate(0, height)
                ctx.rotate(3 * (Math.PI/2))

                if (ui.currentLanguageLayoutDirection() === Qt.RightToLeft) {
                    ctx.textAlign = "start"
                } else {
                    ctx.textAlign = "end"
                }
            }

            ctx.clearRect(0, 0, bgCanvas.height, bgCanvas.width)
            ctx.font = prv.unitTextFont

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

    handle: Item {
        x: root.width - prv.handleWidth
        y: (1 - root.position) * prv.rulerLineHeight
        implicitWidth: prv.handleWidth
        implicitHeight: prv.handleHeight

        MouseArea {
            anchors.fill: parent

            onDoubleClicked: {
                // Double click resets the volume
                root.volumeLevelMoved(0.0)
            }

            // The MouseArea steals mouse press events from the slider.
            // There is really no way to prevent that.
            // (if you set mouse.accepted to false in the onPressed handler,
            // the MouseArea won't receive doubleClick events).
            // So we use this workaround.

            preventStealing: true // Don't let a Flickable steal the mouse

            onPressed: function(mouse) {
                prv.dragStartOffset = mouse.y
            }

            onPositionChanged: function(mouse)  {
                let mousePosInRoot = mapToItem(root, 0, mouse.y - prv.dragStartOffset).y
                let newPosZeroToOne = 1 - mousePosInRoot / prv.rulerLineHeight
                let newPosClamped = Math.max(0.0, Math.min(newPosZeroToOne, 1.0))
                let localNewValue = root.valueAt(newPosClamped)
                let logicalNewValue = convertor.volumeLevelFromLocal(localNewValue)
                root.volumeLevelMoved(logicalNewValue)
            }
        }

        ItemWithDropShadow {
            anchors.fill: parent

            shadow.color: "#33000000"
            shadow.radius: 4
            shadow.verticalOffset: 2

            Rectangle {
                id: handleRect
                anchors.fill: parent
                radius: 2
                color: "white"

                Rectangle {
                    anchors.fill: parent
                    radius: 2

                    color: Utils.colorWithAlpha(ui.theme.popupBackgroundColor, 0.1)
                    border.color: Qt.rgba(0, 0, 0, 0.7)
                    border.width: 1

                    Rectangle {
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.left: parent.left
                        anchors.right: parent.right
                        anchors.leftMargin: 3
                        anchors.rightMargin: 3

                        height: 1
                        radius: height / 2
                        color: "#000000"
                    }
                }
            }
        }
    }

    onMoved: {
        navigation.requestActiveByInteraction()

        var newLevel = convertor.volumeLevelFromLocal(value)
        root.volumeLevelMoved(newLevel)
    }

    Component.onCompleted: {
        bgCanvas.requestPaint()
    }
}
