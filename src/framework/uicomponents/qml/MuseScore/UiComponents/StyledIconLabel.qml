import QtQuick 2.12
import MuseScore.Ui 1.0

StyledTextLabel {
    id: root

    property var iconCode
    property bool isEmpty: iconCode === undefined

    height: isEmpty ? 0 : implicitHeight
    width: isEmpty ? 0 : implicitWidth

    font {
        family: ui.theme.iconsFont.family
        pixelSize: ui.theme.iconsFont.pixelSize
    }

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
