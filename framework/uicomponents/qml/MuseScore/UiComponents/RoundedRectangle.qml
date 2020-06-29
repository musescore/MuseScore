import QtQuick 2.0

Canvas {
    id: root

    property int topLeftRadius: 0
    property int topRightRadius: 0
    property int bottomLeftRadius: 0
    property int bottomRightRadius: 0

    property color color: "#FFFFFF"

    property QtObject border: QtObject {
        property int width: 0
        property color color: "#00000000"
    }

    onPaint: {
        roundRect(0, 0, width, height)
    }

    function roundRect(x, y, w, h)
    {
        var context = root.getContext("2d")
        var r = x + w
        var b = y + h

        context.beginPath()
        context.fillStyle = root.color
        context.strokeStyle = root.border.color
        context.lineWidth = root.border.width

        context.moveTo(x + topLeftRadius, y)
        context.lineTo(r - topRightRadius, y)
        context.quadraticCurveTo(r, y, r, y + topRightRadius)
        context.lineTo(r, b - bottomRightRadius)
        context.quadraticCurveTo(r, b, r - bottomRightRadius, b)
        context.lineTo(x + bottomLeftRadius, b)
        context.quadraticCurveTo(x, b, x, b - bottomLeftRadius)
        context.lineTo(x, y + topLeftRadius)
        context.quadraticCurveTo(x, y, x + topLeftRadius, y)

        context.stroke()
        context.fill()
    }
}
