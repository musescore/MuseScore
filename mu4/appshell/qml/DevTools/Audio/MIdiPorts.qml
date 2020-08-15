import QtQuick 2.7
import QtQuick.Controls 2.2
import MuseScore.UiComponents 1.0
import MuseScore.Midi 1.0

Item {

    MidiPortDevModel {
        id: midiModel

        onOutputDevicesChanged: {
            outputDevicesRepeater.model = 0
            outputDevicesRepeater.model = midiModel.outputDevices()
        }
    }

    Column {
        anchors.left: parent.left
        anchors.right: parent.right
        height: childrenRect.height

        StyledTextLabel {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 8
            height: 40
            verticalAlignment: Text.AlignVCenter
            font.bold: true
            text: "Midi output"
        }

        StyledTextLabel {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: 8
            height: 40
            horizontalAlignment: Text.AlignLeft
            font.bold: true
            text: "Devices:"
        }

        Column {
            anchors.left: parent.left
            anchors.right: parent.right
            height: childrenRect.height

            Repeater {
                id: outputDevicesRepeater
                model: midiModel.outputDevices()
                delegate: Item {

                    anchors.left: parent.left
                    anchors.right: parent.right
                    anchors.leftMargin: 8
                    anchors.rightMargin: 8
                    height: 40

                    StyledTextLabel {
                        id: label
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.left: parent.left
                        width: 240
                        horizontalAlignment: Text.AlignLeft
                        text: modelData.id + " " + modelData.name
                    }

                    FlatButton {
                        id: discBtn
                        anchors.left: label.right
                        anchors.verticalCenter: parent.verticalCenter
                        height: 30
                        text: modelData.action
                        onClicked: midiModel.outputDeviceAction(modelData.id, modelData.action)
                    }

                    StyledTextLabel {
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom
                        anchors.left: discBtn.right
                        anchors.right: parent.right
                        anchors.leftMargin: 8
                        horizontalAlignment: Text.AlignLeft
                        text: modelData.error
                    }
                }
            }
        }
    }
}
