import QtQuick 2.7
import MuseScore.Ui 1.0

Item {

    id: root

    property var iconCode
    property bool isEmpty: iconCode === undefined


    height: isEmpty ? 0 : label.implicitHeight
    width: isEmpty ? 0 : label.implicitWidth

    Text {
        id: label

        anchors.fill: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        font {
            family: "MusescoreIcon"
            pixelSize: 16
        }
        color: ui.theme.buttonText

        text: iconCharCode(root.iconCode)

        function iconCharCode(code) {
            switch (code) {
            case IconCode.AUTO: return "AUTO";
            case IconCode.NONE: return "NONE";
            default: return String.fromCharCode(code)
            }
        }
    }
}
