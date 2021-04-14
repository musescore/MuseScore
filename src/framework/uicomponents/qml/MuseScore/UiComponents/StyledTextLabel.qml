import QtQuick 2.12

Text {
    id: root

    readonly property bool isEmpty: text.length === 0

    color: ui.theme.fontPrimaryColor
    linkColor: ui.theme.linkColor
    opacity: root.enabled ? 1.0 : ui.theme.itemOpacityDisabled

    elide: Text.ElideRight
    verticalAlignment: Text.AlignVCenter
    horizontalAlignment: Text.AlignHCenter

    font {
        family: ui.theme.bodyFont.family
        pixelSize: ui.theme.bodyFont.pixelSize
    }

    onLinkActivated: Qt.openUrlExternally(link)

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.NoButton
        cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
    }
}
