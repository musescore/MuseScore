import QtQuick 2.7
import MuseScore.UiComponents 1.0
import MuseScore.Audio 1.0

Rectangle {

    color: "#15C39A"

    AudioEngineDevTools {
        id: devtools
    }

    Column {
        anchors.fill: parent

        Row {
            anchors.left:  parent.left
            anchors.right: parent.right
            height:  40
            FlatButton {
                text: "Play Sine"
                width: 80
                onClicked: devtools.playSine()
            }

            FlatButton {
                text: "Stop Sine"
                width: 80
                onClicked: devtools.stopSine()
            }
        }

        Row {
            anchors.left:  parent.left
            anchors.right: parent.right
            height:  40
            FlatButton {
                text: "Play Midi"
                width: 80
                onClicked: devtools.playMidi()
            }

            FlatButton {
                text: "Stop Midi"
                width: 80
                onClicked: devtools.stopMidi()
            }
        }
    }
}
