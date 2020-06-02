import QtQuick 2.0

Text {
    id: root

    color: globalStyle ? globalStyle.buttonText : "#000000"
    opacity: root.enabled ? 1.0 : 0.3

    elide: Text.ElideRight
    verticalAlignment: Text.AlignVCenter
    horizontalAlignment: Text.AlignHCenter

    font {
        family: globalStyle ? globalStyle.font.family : "FreeSerif"
        pixelSize: globalStyle ? globalStyle.font.pixelSize : 12
    }
}
