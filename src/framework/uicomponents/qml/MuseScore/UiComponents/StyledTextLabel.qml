import QtQuick 2.12

Text {
    id: root

    color: ui.theme.fontPrimaryColor
    opacity: root.enabled ? 1.0 : ui.theme.itemOpacityDisabled

    elide: Text.ElideRight
    verticalAlignment: Text.AlignVCenter
    horizontalAlignment: Text.AlignHCenter

    font {
        family: ui.theme.bodyFont.family
        pixelSize: ui.theme.bodyFont.pixelSize
    }
}
