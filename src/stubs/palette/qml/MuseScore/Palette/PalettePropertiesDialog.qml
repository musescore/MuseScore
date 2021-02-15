import QtQuick 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

QmlDialog {
    width: 280
    height: 370

    title: qsTrc("palette", "Palette Properties")

    property var properties

    Rectangle {
        anchors.fill: parent

        color: ui.theme.backgroundPrimaryColor

        StyledTextLabel {
            anchors.centerIn: parent
            text: "Palette Properties Dialog Stub"
        }
    }
}
