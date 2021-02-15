import QtQuick 2.7
import QtGraphicalEffects 1.0

Image {
    id: root

    property alias url: root.source
    property alias side: root.width

    height: width

    fillMode: Image.PreserveAspectCrop

    layer.enabled: true
    layer.effect: OpacityMask {
        maskSource: Rectangle {
            width: root.width
            height: root.height
            radius: width / 2
            visible: false
        }
    }
}
