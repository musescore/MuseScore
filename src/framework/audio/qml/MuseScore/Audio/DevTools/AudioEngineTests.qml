import QtQuick 2.7
import MuseScore.UiComponents 1.0
import MuseScore.Audio 1.0
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3

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
            height: 40

            ComboBox {
                id: pluginSelector
                model: devtools.devices
                width: parent.width / 2
                currentIndex: devtools.devices.indexOf(devtools.device())
                onCurrentValueChanged: devtools.selectDevice(currentValue)
            }
        }

        Row {
            anchors.left:  parent.left
            anchors.right: parent.right
            height:  40
            spacing: 8
            FlatButton {
                text: "Play Sequencer Midi"
                width: 120
                onClicked: devtools.playSequencerMidi()
            }

            FlatButton {
                text: "Stop Source Midi"
                width: 120
                onClicked: devtools.stopSequencerMidi()
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
                text: "Play"
                width: 120
                onClicked: devtools.play()
            }

            FlatButton {
                text: "Stop"
                width: 120
                onClicked: devtools.stop()
            }

            FlatButton {
                text: "Set loop"
                width: 120
                onClicked: devtools.setLoop(1000, 4000)
            }

            FlatButton {
                text: "unset loop"
                width: 120
                onClicked: devtools.unsetLoop()
            }
        }

        Row {
            anchors.left:  parent.left
            anchors.right: parent.right
            height:  40
            spacing: 8
            FlatButton {
                text: "Open audio file"
                width: 120
                onClicked: devtools.openAudio()
            }

            FlatButton {
                text: "Close audio file"
                width: 120
                onClicked: devtools.closeAudio()
            }
        }

        Row {
            anchors.left:  parent.left
            anchors.right: parent.right
            height:  60
            Text {
                text: "Frame:" + parseInt(devtools.time)  + " : " + parseInt(((devtools.time * 1000) % 1000) * 3 / 100)
                font.bold: true
                font.pixelSize: 60
                anchors.left:  parent.left
                anchors.right: parent.right
            }
        }

        RowLayout {
            spacing: 2
            anchors.left:  parent.left
            anchors.right: parent.right

            ColumnLayout {
                spacing: 2
                Layout.fillHeight: true
                Layout.preferredWidth: 120
                Layout.minimumWidth: 124
                Layout.alignment: Qt.AlignCenter

                Label {
                    width: 120
                    text: "noise"
                    font.pixelSize: 22
                }

                FlatButton {
                    text: "Play"
                    width: 120
                    onClicked: devtools.playNoise()
                }

                FlatButton {
                    text: "Stop"
                    width: 120
                    onClicked: devtools.stopNoise()
                }

                Switch {
                    text: qsTr("eq")
                    onPositionChanged: devtools.enableNoiseEq(position > 0.5);
                }

                Switch {
                    text: qsTr("mute")
                    onPositionChanged: devtools.setMuteNoise(position > 0.5);
                }

                Slider {
                    from: -100
                    value: 0
                    to: 100
                    Layout.preferredWidth: 120
                    onPositionChanged: devtools.setBalanceNoise(2 * position - 1) //from 0..1 to -1..1
                }

                Slider {
                    from: 0
                    value: 100
                    to: 100
                    orientation: Qt.Vertical
                    onPositionChanged: devtools.setLevelNoise(position)
                }
            }

            ColumnLayout {
                spacing: 2
                Layout.fillHeight: true
                Layout.preferredWidth: 120
                Layout.minimumWidth: 124
                Layout.alignment: Qt.AlignCenter

                Label {
                    width: 120
                    text: "sin"
                    font.pixelSize: 22
                }

                FlatButton {
                    text: "Play"
                    width: 120
                    onClicked: devtools.playSine()
                }

                FlatButton {
                    text: "Stop"
                    width: 120
                    onClicked: devtools.stopSine()
                }

                Switch {
                    text: qsTr("eq")
                    enabled: false
                }

                Switch {
                    text: qsTr("mute")
                    onPositionChanged: devtools.setMuteSine(position > 0.5);
                }

                Slider {
                    from: -100
                    value: 0
                    to: 100
                    width: 120
                    Layout.preferredWidth: 120
                    onPositionChanged: devtools.setBalanceSine(2 * position - 1) //from 0..1 to -1..1
                }

                Slider {
                    from: 0
                    value: 100
                    to: 100
                    orientation: Qt.Vertical
                    onPositionChanged: devtools.setLevelSine(position)
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
            }
        }
    }
}
