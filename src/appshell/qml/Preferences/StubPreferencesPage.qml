import QtQuick 2.15

import MuseScore.UiComponents 1.0

Item {
    id: root

    Rectangle {
        anchors.fill: parent

        color: ui.theme.backgroundSecondaryColor

        StyledTextLabel {
            anchors.centerIn: parent
            text: qsTrc("appshell", "Not implemented")
        }
    }
}
