import QtQuick 2.15
import QtQuick.Controls 2.15

import MuseScore.Ui 1.0

Dial {
    id: root

    width: internal.radius * 2
    height: width

    from: -100
    to: 100
    value: 0

    QtObject {
        id: internal

        property real radius: 24
        readonly property bool reversed: angle < 0

        property real handlerHeight: 8
        property real handlerWidth: 2

        property real outerArcLineWidth: 5
        property real innerArcLineWidth: 3

        property color valueArcColor: ui.theme.accentColor
        property color outerArcColor: {
            var oc = ui.theme.buttonColor
            return Qt.rgba(oc.r, oc.g, oc.b, 0.7)
        }

        property color innerArcColor: {
            var ic = ui.theme.fontPrimaryColor
            return Qt.rgba(ic.r, ic.g, ic.b, 0.5)
        }
    }

    background: Canvas {
        id: backgroundCanvas

        width: internal.radius * 2
        height: width

        antialiasing: true
        renderStrategy: Canvas.Threaded
        renderTarget: Canvas.FramebufferObject

        onPaint: {
            var ctx = root.context

            if (!ctx) {
                ctx = getContext("2d")
                ctx.lineCap = "squared"
            }

            ctx.clearRect(0, 0, canvasSize.width, canvasSize.height)
            ctx.lineWidth = internal.outerArcLineWidth

            ctx.strokeStyle = internal.outerArcColor
            ctx.beginPath()
            ctx.arc(width/2, height/2, internal.radius - internal.outerArcLineWidth, -140 * (Math.PI/180) - Math.PI/2, 140 * (Math.PI/180) - Math.PI/2, false)
            ctx.stroke()

            ctx.strokeStyle = internal.valueArcColor
            ctx.beginPath()
            ctx.arc(width/2, height/2, internal.radius - internal.outerArcLineWidth, -Math.PI/2, angle * (Math.PI/180) - Math.PI/2, internal.reversed)
            ctx.stroke()

            ctx.lineWidth = internal.innerArcLineWidth
            ctx.strokeStyle = internal.innerArcColor
            ctx.beginPath()
            ctx.arc(width/2, height/2, internal.radius - (internal.outerArcLineWidth + internal.innerArcLineWidth), 0, Math.PI * 2, false)
            ctx.stroke()
        }
    }

    handle: Item {
        id: handleItem

        x: internal.radius - internal.outerArcLineWidth - internal.innerArcLineWidth + internal.handlerWidth / 2
        y: internal.outerArcLineWidth + internal.innerArcLineWidth + 2

        height: internal.radius - internal.outerArcLineWidth - internal.innerArcLineWidth
        width: internal.handlerWidth

        Rectangle {
            height: internal.handlerHeight
            width: internal.handlerWidth
            radius: internal.handlerWidth / 2

            color: ui.theme.fontPrimaryColor
            antialiasing: true
        }

        rotation: root.angle
    }

    onValueChanged: {
        backgroundCanvas.requestPaint()
    }

    Component.onCompleted: {
        backgroundCanvas.requestPaint()
    }
}
