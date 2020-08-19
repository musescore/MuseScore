import QtQuick 2.9
import MuseScore.Ui 1.0

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
    color: ui.theme ? ui.theme.fontPrimaryColor : "#CECECE"

    text: iconCharCode(iconCode)

    function iconCharCode(code) {
        var result = 0

        switch (code) {
        case IconCode.AUTO: result = "AUTO"; break
        case IconCode.NONE: result = "NONE"; break
        case IconCode.CUSTOM: result = "Custom"; break
        default: result = String.fromCharCode(code); break
        }

        return result
    }
}
