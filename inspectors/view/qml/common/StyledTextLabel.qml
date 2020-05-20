import QtQuick 2.0

Text {
    id: root

    color: globalStyle.buttonText
    opacity: root.enabled ? 1.0 : 0.3

    elide: Text.ElideRight
    verticalAlignment: Text.AlignVCenter
    horizontalAlignment: Text.AlignHCenter

    font {
        family: globalStyle.font.family
        pixelSize: globalStyle.font.pixelSize
    }
}
