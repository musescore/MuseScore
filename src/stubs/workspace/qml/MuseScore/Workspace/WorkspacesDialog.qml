import QtQuick 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

QmlDialog {
    id: root

    height: 600
    width: 1024

    Rectangle {
        anchors.fill: parent
        color: ui.theme.backgroundPrimaryColor

        StyledTextLabel {
            anchors.centerIn: parent
            text: "Workspaces Dialog Stub"
        }
    }
}
