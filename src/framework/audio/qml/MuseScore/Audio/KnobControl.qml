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
import MuseScore.UiComponents 1.0

Dial {
    id: root

    property int valueScale: 100

    property alias navigation: navCtrl

    property real radius: 16

    implicitWidth: root.radius * 2
    implicitHeight: implicitWidth

    width: implicitWidth
    height: width

    wheelEnabled: true

    from: -root.valueScale
    to: root.valueScale
    value: 0

    signal newValueRequested(int newValue)
    signal increaseRequested()
    signal decreaseRequested()

    QtObject {
        id: prv

        readonly property bool reversed: root.angle < 0

        readonly property real handlerHeight: 8
        readonly property real handlerWidth: 2

        readonly property real outerArcLineWidth: 3
        readonly property real innerArcLineWidth: 2

        readonly property color valueArcColor: ui.theme.accentColor
        readonly property color outerArcColor: Utils.colorWithAlpha(ui.theme.buttonColor, 0.7)
        readonly property color innerArcColor: Utils.colorWithAlpha(ui.theme.fontPrimaryColor, 0.5)

        property int initialValue: 0
        property real dragStartX: 0
        property real dragStartY: 0

        onValueArcColorChanged: { backgroundCanvas.requestPaint() }
        onOuterArcColorChanged: { backgroundCanvas.requestPaint() }
        onInnerArcColorChanged: { backgroundCanvas.requestPaint() }
    }

    NavigationControl {
        id: navCtrl
        name: root.objectName != "" ? root.objectName : "KnobControl"
        enabled: root.enabled && root.visible

        accessible.role: MUAccessible.Range
        accessible.visualItem: root

        accessible.value: root.value
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
        id: backgroundCanvas

        width: root.radius * 2
        height: width

        antialiasing: true

        NavigationFocusBorder {
            navigationCtrl: navCtrl
        }

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
            ctx.arc(width/2, height/2, root.radius - prv.outerArcLineWidth/2, -140 * (Math.PI/180) - Math.PI/2, 140 * (Math.PI/180) - Math.PI/2, false)
            ctx.stroke()

            ctx.strokeStyle = prv.valueArcColor
            ctx.beginPath()
            ctx.arc(width/2, height/2, root.radius - prv.outerArcLineWidth/2, -Math.PI/2, root.angle * (Math.PI/180) - Math.PI/2, prv.reversed)
            ctx.stroke()

            ctx.lineWidth = prv.innerArcLineWidth
            ctx.strokeStyle = prv.innerArcColor
            ctx.beginPath()
            ctx.arc(width/2, height/2, root.radius - (prv.outerArcLineWidth + prv.innerArcLineWidth/2), 0, Math.PI * 2, false)
            ctx.stroke()
        }
    }

    handle: Rectangle {
        x: root.radius - prv.handlerWidth / 2
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

    onMoved: {
        navigation.requestActiveByInteraction()

        newValueRequested(value)
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onDoubleClicked: {
            root.newValueRequested(0)
        }

        // The MouseArea steals mouse press events from the slider.
        // There is really no way to prevent that.
        // (if you set mouse.accepted to false in the onPressed handler,
        // the MouseArea won't receive doubleClick events).
        // So we have to reimplement the dragging behaviour.
        // That gives us the opportunity to do it better than Qt.
        // We will allow both dragging vertically and horizontally.

        preventStealing: true // Don't let a Flickable steal the mouse

        onPressed: function(mouse) {
            prv.initialValue = root.value
            prv.dragStartX = mouse.x
            prv.dragStartY = mouse.y
        }

        onPositionChanged: function(mouse)  {
            let dx = mouse.x - prv.dragStartX
            let dy = mouse.y - prv.dragStartY
            let dist = Math.sqrt(dx * dx + dy * dy)
            let sgn = (dy < dx) ? 1 : -1
            let newValue = prv.initialValue + dist * sgn
            let bounded = Math.max(root.from, Math.min(newValue, root.to))
            root.newValueRequested(bounded)
        }

        // We also listen for wheel events here, but for a different reason:
        // Qml Dial has a bug that it doesn't emit moved() when the value is changed through a wheel event.
        // So when we see a wheel event, we let the dial handle it, but we will account for emitting the signal.
        onWheel: function(wheel) {
            wheel.accepted = false
            Qt.callLater(function() { root.newValueRequested(Math.round(value)) })
        }
    }
}
