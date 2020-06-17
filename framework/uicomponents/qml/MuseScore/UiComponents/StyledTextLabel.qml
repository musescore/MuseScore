import QtQuick 2.0

Text {
    id: root

    color: ui.theme ? ui.theme.buttonText : "#000000"
    opacity: root.enabled ? 1.0 : 0.3

    elide: Text.ElideRight
    verticalAlignment: Text.AlignVCenter
    horizontalAlignment: Text.AlignHCenter

    font {
        family: ui.theme ? ui.theme.font.family : "FreeSerif"
        pixelSize: ui.theme ? ui.theme.font.pixelSize : 12
    }
}
