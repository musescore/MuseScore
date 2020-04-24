import QtQuick 2.9
import MuseScore.Inspectors 3.3

Text {
    id: root

    property var iconCode
    property bool isEmpty: iconCode === undefined

    height: isEmpty ? 0 : implicitHeight
    width: isEmpty ? 0 : implicitWidth

    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter

    font {
        family: "MusescoreIcon"
        pixelSize: 16
    }
    color: globalStyle.buttonText

    text: iconCharCode(iconCode)

    function iconCharCode(code) {
        switch (code) {
        case IconNameTypes.AUTO: return "AUTO";
        case IconNameTypes.NONE: return "NONE";
        default: return String.fromCharCode(code)
        }
    }
}
