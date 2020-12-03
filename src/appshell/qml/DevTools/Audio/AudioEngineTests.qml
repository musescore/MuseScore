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
            spacing: 8
            FlatButton {
                text: "Play Sine"
                width: 120
                onClicked: devtools.playSine()
            }

            FlatButton {
                text: "Stop Sine"
                width: 120
                onClicked: devtools.stopSine()
            }
        }

        Row {
            anchors.left:  parent.left
            anchors.right: parent.right
            height:  40
            spacing: 8
            FlatButton {
                text: "Play Source Midi"
                width: 120
                onClicked: devtools.playSourceMidi()
            }

            FlatButton {
                text: "Stop Source Midi"
                width: 120
                onClicked: devtools.stopSourceMidi()
            }
        }

        Row {
            anchors.left:  parent.left
            anchors.right: parent.right
            height:  40
            spacing: 8
            FlatButton {
                text: "Play Player Midi"
                width: 120
                onClicked: devtools.playPlayerMidi()
            }

            FlatButton {
                text: "Stop Player Midi"
                width: 120
                onClicked: devtools.stopPlayerMidi()
            }
        }

        Row {
            anchors.left:  parent.left
            anchors.right: parent.right
            height:  40
            spacing: 8
            FlatButton {
                text: "Play Notation"
                width: 120
                onClicked: devtools.playNotation()
            }

            FlatButton {
                text: "Stop Notation"
                width: 120
                onClicked: devtools.stopNotation()
            }
        }
    }
}
