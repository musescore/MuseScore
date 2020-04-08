import QtQuick 2.1
import QtGraphicalEffects 1.0

Item {
    property alias source: img.source
    property alias color: colorOverlay.color//globalStyle.buttonText

    Image {
        id: img
        anchors.fill: parent
        fillMode: Image.PreserveAspectFit
        source: parent.source
    }
    ColorOverlay {
        id: colorOverlay

        anchors.fill: img
        source: img
        color: "#000000"
    }
}
