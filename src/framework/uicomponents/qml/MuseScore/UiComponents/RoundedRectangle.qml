import QtQuick 2.0

Canvas {
    id: root

    property int radius: 0

    property int topLeftRadius: radius
    property int topRightRadius: radius
    property int bottomLeftRadius: radius
    property int bottomRightRadius: radius

    property color color: "#FFFFFF"

    property Border border: Border {}

    onColorChanged: {
        requestPaint()
    }

    onBorderChanged: {
        requestPaint()
    }

    onTopLeftRadiusChanged: {
        requestPaint()
    }

    onTopRightRadiusChanged: {
        requestPaint()
    }

    onBottomRightRadiusChanged: {
        requestPaint()
    }

    onBottomLeftRadiusChanged: {
        requestPaint()
    }

    onPaint: {
        roundRect(0, 0, width, height)
    }

    function roundRect(x1, y1, x2, y2)
    {
        var context = root.getContext("2d")
        var b = border.width / 2
        var x1Inner = x1 + b
        var y1Inner = y1 + b
        var x2Inner = x2 - b
        var y2Inner = y2 - b

        context.beginPath()
        context.fillStyle = root.color
        context.strokeStyle = root.border.color
        context.lineWidth = root.border.width

        context.moveTo(x1 + topLeftRadius, y1Inner)

        if (topRightRadius == 0) {
            context.lineTo(x2Inner, y1Inner)
        } else {
            context.lineTo(x2 - topRightRadius, y1Inner)
            context.quadraticCurveTo(x2Inner, y1Inner, x2Inner, y1 + topRightRadius)
        }

        if (bottomRightRadius == 0) {
            context.lineTo(x2Inner, y2Inner)
        } else {
            context.lineTo(x2Inner, y2 - bottomRightRadius)
            context.quadraticCurveTo(x2Inner, y2Inner, x2 - bottomRightRadius, y2Inner)
        }

        if (bottomLeftRadius == 0) {
            context.lineTo(x1Inner, y2Inner)
        } else {
            context.lineTo(x1 + bottomLeftRadius, y2Inner)
            context.quadraticCurveTo(x1Inner, y2Inner, x1Inner, y2 - bottomLeftRadius)
        }

        if (topLeftRadius == 0) {
            context.lineTo(x1Inner, y1Inner)
        } else {
            context.lineTo(x1Inner, y1 + topLeftRadius)
            context.quadraticCurveTo(x1Inner, y1Inner, x1 + topLeftRadius, y1Inner)
        }

        context.stroke()
        context.fill()
    }
}
