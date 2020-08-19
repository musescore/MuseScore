import QtQuick 2.1
import QtGraphicalEffects 1.0

Item {
    id: root

    property alias icon: image.source
    property alias sourceSize: image.sourceSize
    property alias color: colorOverlay.color
    property var pixelSize: 16

    implicitHeight: root.icon == "" ? 0 : pixelSize
    implicitWidth: root.icon == "" ? 0 : pixelSize

    Image {
        id: image

        anchors.centerIn: parent

        height: pixelSize
        width: implicitWidth

        fillMode: Image.PreserveAspectFit
    }

    ColorOverlay {
        id: colorOverlay

        anchors.fill: image
        source: image
        color: ui.theme.fontPrimaryColor
    }
}
