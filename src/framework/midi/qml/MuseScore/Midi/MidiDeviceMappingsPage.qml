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

    EditMidiActionDialog {
        id: editActionDialog

        actionTitle: "Rewind"
        actionIcon: IconCode.REWIND
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
            keyTitle: qsTrc("midi", "action")
            valueRoleName: "status"
            valueTitle: qsTrc("midi", "status")
            iconRoleName: "icon"
            valueEnabledRoleName: "enabled"

            model: mappingsModel

            onDoubleClicked: {
                editActionDialog.open()
            }
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
