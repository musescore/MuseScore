import QtQuick 2.0

Text {
    id: root

    color: ui.theme.fontPrimaryColor
    opacity: root.enabled ? 1.0 : 0.3

    elide: Text.ElideRight
    verticalAlignment: Text.AlignVCenter
    horizontalAlignment: Text.AlignHCenter

    font {
        family: ui.theme.font.family
        pixelSize: ui.theme.font.pixelSize
    }
}
