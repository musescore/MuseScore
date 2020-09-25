import QtQuick 2.9
import QtQuick.Controls 2.2

import MuseScore.Ui 1.0

ScrollBar {
    id: root

    property alias trackEnabled: background.visible

    width: 10

    contentItem: Rectangle {
        radius: 8

        color: ui.theme.fontPrimaryColor
        opacity: root.pressed ? 0.7 : 0.3
    }

    Rectangle {
        id: background

        anchors.fill: parent
        z: -1

        color: ui.theme.backgroundPrimaryColor

        visible: false
    }
}
