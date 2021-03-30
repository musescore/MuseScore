import QtQuick 2.15
import QtQuick.Layouts 1.12

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Midi 1.0

Item {
    id: root

    function apply() {
        return mappingsModel.apply()
    }

    MidiDeviceMappingsModel {
        id: mappingsModel

        selection: view.selection
    }

    Component.onCompleted: {
        mappingsModel.load()
    }

    ColumnLayout {
        anchors.fill: parent

        spacing: 20

        CheckBox {
            text: qsTrc("midi", "MIDI Remote Control")
            font: ui.theme.bodyBoldFont

            checked: mappingsModel.useRemoteControl

            onClicked:  {
                mappingsModel.useRemoteControl = !checked
            }
        }

        ValueList {
            id: view

            Layout.fillWidth: true
            Layout.fillHeight: true

            enabled: mappingsModel.useRemoteControl
            readOnly: true

            keyRoleName: "title"
            keyTitle: qsTrc("midi", "ACTION")
            valueRoleName: "status"
            valueTitle: qsTrc("midi", "STATUS")
            iconRoleName: "icon"

            model: mappingsModel
        }

        Row {
            enabled: mappingsModel.useRemoteControl

            Layout.alignment: Qt.AlignRight

            spacing: 8

            FlatButton {
                text: qsTrc("midi", "Assign MIDI mapping...")

                onClicked: {

                }
            }

            FlatButton {
                width: 100

                text: qsTrc("global", "Clear")

                onClicked: {
                    mappingsModel.clearSelectedActions()
                }
            }

            FlatButton {
                width: 100

                text: qsTrc("global", "Clear all")

                onClicked: {
                    mappingsModel.clearAllActions()
                }
            }
        }
    }
}
