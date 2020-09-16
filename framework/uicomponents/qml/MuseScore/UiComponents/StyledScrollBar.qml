import QtQuick 2.9
import QtQuick.Controls 2.2

import MuseScore.Ui 1.0

ScrollBar {
    id: root

    width: 16

    contentItem: Rectangle {
        radius: 8

        color: ui.theme.popupBackgroundColor
    }
}
