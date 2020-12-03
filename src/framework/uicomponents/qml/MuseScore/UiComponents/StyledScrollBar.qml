import QtQuick 2.9
import QtQuick.Controls 2.2

import MuseScore.Ui 1.0

ScrollBar {
    id: root

    width: 10

    contentItem: Rectangle {
        radius: 5
        color: ui.theme.fontPrimaryColor
        opacity: root.pressed ? 0.7 : 0.3
        visible: root.active
    }
}
