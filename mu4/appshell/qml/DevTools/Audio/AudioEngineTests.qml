import QtQuick 2.7
import MuseScore.UiComponents 1.0
import MuseScore.Audio 1.0

Rectangle {

    color: "#15C39A"

    AudioEngineDevTools {
        id: devtools
    }

    Row {
        anchors.left:  parent.left
        anchors.right: parent.right
        height:  40
        FlatButton {
            text: "Play"
            width: 80
            onClicked: devtools.playSine()
        }

        FlatButton {
            text: "Stop"
            width: 80
            onClicked: devtools.stopSine()
        }
    }
}
