import QtQuick 2.15

import MuseScore.UiComponents 1.0

Column {
    id: root

    property alias icon: iconLabel.iconCode
    property alias text: textLabel.text

    anchors.horizontalCenter: parent.horizontalCenter
    anchors.verticalCenter: parent.verticalCenter

    spacing: 10

    StyledIconLabel {
        id: iconLabel

        anchors.horizontalCenter: parent.horizontalCenter
        height: 50

        font.pixelSize: 64
        renderType: Text.NativeRendering
    }

    StyledTextLabel {
        id: textLabel

        anchors.horizontalCenter: parent.horizontalCenter
    }
}
