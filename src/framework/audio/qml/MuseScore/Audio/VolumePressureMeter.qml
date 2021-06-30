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

    width: 24
    height: 140

    renderStrategy: Canvas.Threaded
    renderTarget: Canvas.FramebufferObject

    QtObject {
        id: privateProperties

        property var gradient: null
        property int overloadHeight: 4
        property real pixelsPerUnit: height / minDisplayedVolumePressure
    }

    onPaint: {
        var ctx = root.context

        if (!ctx) {
            ctx = getContext("2d")
            ctx.translate(0, height)
            ctx.rotate(3 * (Math.PI/2))
        }

        ctx.clearRect(0, 0, height, width)

        ctx.fillStyle = "#4D4D4D"
        ctx.fillRect(0, 0, height - privateProperties.overloadHeight, width)

        ctx.fillStyle = "#666666"
        ctx.fillRect(height - privateProperties.overloadHeight, 0, height, width)

        if (!privateProperties.gradient) {
            privateProperties.gradient = ctx.createLinearGradient(0, 0, height - privateProperties.overloadHeight, width)
            privateProperties.gradient.addColorStop(0.0, "#26E386")
            privateProperties.gradient.addColorStop(0.55, "#CBED41")
            privateProperties.gradient.addColorStop(0.80, "#FC8226")
        }

        var value = height - (currentVolumePressure * privateProperties.pixelsPerUnit)

        ctx.fillStyle = privateProperties.gradient
        ctx.fillRect(0, 0, value, width)
    }

    onCurrentVolumePressureChanged: {
        requestPaint()
    }

    Component.onCompleted: {
        requestPaint()
    }
}
