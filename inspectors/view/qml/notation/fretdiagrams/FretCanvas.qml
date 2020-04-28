import QtQuick 2.0
import QtQuick.Controls 2.2
import MuseScore.Inspectors 3.3
import "../../common/"

FocusableItem {
    id: root
    height: 300
    width: parent.width

    property QtObject model: null

    property int strings: model ? model.strings.value : 6
    property int frets: model ? model.frets.value : 4
    property int fretOffset: model ? model.offset.value : 0
    property bool showNut: model ? model.showNut.value : true
    property int numPos: model ? model.numPos.value : 0 // TODO: make this have effect
    property bool antialiasing: true

    readonly property real mag: 1.5
    readonly property real spatium: 20.0 * mag
    readonly property real lineWidth1: spatium * 0.08
    property real lineWidth2: (fretOffset || !showNut) ? lineWidth1 : spatium * 0.2
    readonly property real stringDist: spatium * 0.7
    readonly property real fretDist: spatium * 0.8
    readonly property real dotD: stringDist * 0.6 + lineWidth1

    property real diagramWidth: (strings - 1) * stringDist
    property real diagramHeight: (frets * fretDist) + fretDist * 0.5

    Component.onCompleted: {
        model.onSelectionChanged.connect(frame.requestPaint);
        model.onSelectionChanged.connect(canvas.requestPaint);
    }

    FontLoader {
        id: freeSans
        source: "../../../../../fonts/FreeSans.ttf"
    }

    readonly property string basicFont: "%1px %2"

    Canvas {
        id: frame
        anchors.fill: parent
        antialiasing: root.antialiasing

        onPaint: {
            var ctx = getContext("2d");
            ctx.reset();

            ctx.strokeStyle = Qt.rgba(0, 0, 0, 1);

            ctx.translate((width - diagramWidth) * 0.5, (height - diagramHeight) * 0.5);
            ctx.save();

            ctx.lineWidth = lineWidth2;
            ctx.beginPath();
            ctx.moveTo(-lineWidth1 * 0.5, -lineWidth1 * 0.5);
            ctx.lineTo((strings - 1) * stringDist + lineWidth1 * 0.5, -lineWidth1 * 0.5);
            ctx.stroke();

            ctx.lineWidth = lineWidth1;
            ctx.beginPath();
            for (var i = 0; i < strings; i++) {
                ctx.moveTo(i * stringDist, fretOffset ? -spatium * 0.2 : 0.0);
                ctx.lineTo(i * stringDist, (frets + 0.5) * fretDist);
            }
            for (var j = 1; j <= frets; j++) {
                ctx.moveTo(0, j * fretDist);
                ctx.lineTo((strings - 1) * stringDist, j * fretDist);
            }
            ctx.stroke();

            if (fretOffset) {
                ctx.fillStyle = Qt.rgba(0, 0, 0, 1);
                var fretNumMag = 2.0; // TODO: get the value from Sid::fretNumMag
                ctx.font = basicFont.arg(18.0 * mag * fretNumMag).arg(freeSans.name);
                ctx.textAlign = "right";
                ctx.textBaseline = "middle";
                ctx.fillText("%1".arg(fretOffset + 1), -stringDist * .4, fretDist * 0.5);
            }
        }
    }

    Canvas {
        id: canvas
        anchors.fill: parent
        antialiasing: root.antialiasing

        function drawMarker(ctx, i, markerType) {
            ctx.font = basicFont.arg(18.0 * mag).arg(freeSans.name);
            ctx.textAlign = "center";
            ctx.textBaseline = "bottom";
            var marker = (markerType === FretDiagramTypes.MARKER_CIRCLE) ? 'O' : 'X';
            ctx.fillText(marker, stringDist * i, -fretDist * 0.1);
        }

        function drawDot(ctx, s, f, dotType) {
            ctx.beginPath();
            switch (dotType) {
                case FretDiagramTypes.DOT_CROSS:
                    ctx.beginPath();
                    ctx.moveTo(s * stringDist - dotD * 0.5, fretDist * 0.5 + f * fretDist - dotD * 0.5);
                    ctx.lineTo(s * stringDist + dotD * 0.5, fretDist * 0.5 + f * fretDist + dotD * 0.5);
                    ctx.moveTo(s * stringDist + dotD * 0.5, fretDist * 0.5 + f * fretDist - dotD * 0.5);
                    ctx.lineTo(s * stringDist - dotD * 0.5, fretDist * 0.5 + f * fretDist + dotD * 0.5);
                    ctx.stroke();
                    break;
                case FretDiagramTypes.DOT_SQUARE:
                    ctx.rect(s * stringDist - dotD * 0.5, fretDist * 0.5 + f * fretDist - dotD * 0.5, dotD, dotD);
                    ctx.stroke();
                    break;
                case FretDiagramTypes.DOT_TRIANGLE:
                    ctx.beginPath();
                    ctx.moveTo(s * stringDist - dotD * 0.5, fretDist * 0.5 + f * fretDist + dotD * 0.5);
                    ctx.lineTo(s * stringDist, fretDist * 0.5 + f * fretDist - dotD * 0.5);
                    ctx.lineTo(s * stringDist + dotD * 0.5, fretDist * 0.5 + f * fretDist + dotD * 0.5);
                    ctx.closePath();
                    ctx.stroke();
                    break;
                case FretDiagramTypes.DOT_NORMAL:
                default:
                    ctx.ellipse(s * stringDist - dotD * 0.5, fretDist * 0.5 + f * fretDist - dotD * 0.5, dotD, dotD);
                    ctx.stroke();
                    ctx.fill();
                    break;
            }
        }

        function drawBarre(ctx, f, startString, endString) {
            if (startString === -1)
                return;
            var x1 = stringDist * startString;
            var x2 = (endString === -1) ? stringDist * (strings - 1)  : stringDist * endString;
            var y  = fretDist * 0.5 + fretDist * f;
            ctx.lineWidth = dotD * (model ? model.barreLineWidth() : 1.0);
            ctx.lineCap = "round";
            ctx.beginPath();
            ctx.moveTo(x1, y);
            ctx.lineTo(x2, y);
            ctx.stroke();
        }

        onPaint: {
            var ctx = getContext("2d");
            ctx.reset();

            ctx.strokeStyle = Qt.rgba(0, 0, 0, 1);
            ctx.fillStyle = Qt.rgba(0, 0, 0, 1);
            ctx.lineWidth = lineWidth1;

            ctx.translate((width - diagramWidth) * 0.5, (height - diagramHeight) * 0.5);
            ctx.save();

            for (var s1 = 0; s1 < strings; s1++) {
                var m = model ? model.marker(s1) : 0;
                if (m !== 0)
                    drawMarker(ctx, s1, m);
            }

            for (var s2 = 0; s2 < strings; s2++) {
                for (var f2 = 1; f2 <= frets; f2++) {
                    var d = model ? model.dot(s2, f2) : -1;
                    if (d !== -1)
                        drawDot(ctx, s2, f2 - 1, d);
                }
            }

            for (var f3 = 1; f3 <= frets; f3++) {
                var barreExists = model ? model.barreExists(f3) : false;
                if (barreExists) {
                    var ss = model ? model.barreStartString(f3) : -1;
                    var es = model ? model.barreEndString(f3)   : -1;
                    drawBarre(ctx, f3 - 1, ss, es);
                }
            }
        }
    }
}
