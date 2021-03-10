import QtQuick 2.7
import MuseScore.UiComponents 1.0
import MuseScore.Autobot 1.0

Rectangle {

    color: "#edd400"

    AutobotModel {
        id: autobot
    }

    Row {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.margins: 32
        height: 24
        spacing: 8

        FlatButton {
            width: 64
            text: "Run"
            onClicked: autobot.run()
        }

        FlatButton {
            width: 64
            text: "Stop"
            onClicked: autobot.stop()
        }
    }
}
