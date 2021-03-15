import QtQuick 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

Rectangle {
    color: ui.theme.backgroundPrimaryColor

    width: 220
    height: parent.height

    SearchField {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 12
    }
}
