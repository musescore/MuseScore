import QtQuick 2.15
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Midi 1.0

Dialog {
    id: root

    signal mapToValueRequested(int value)

    title: qsTrc("midi", "MIDI Remote Control")

    height: 220
    width: 538

    standardButtons: Dialog.NoButton

    function startEdit(action) {
        model.load(action.mappedValue)
        open()

        actionNameLabel.text = action.title
        actionIconLabel.iconCode = action.icon
    }

    EditMidiMappingModel {
        id: model
    }

    Rectangle {
        anchors.fill: parent

        color: ui.theme.backgroundPrimaryColor

        Column {
            anchors.fill: parent
            anchors.margins: 8

            spacing: 24

            Row {
                anchors.horizontalCenter: parent.horizontalCenter

                spacing: 8

                StyledIconLabel {
                    id: actionIconLabel
                }

                StyledTextLabel {
                    id: actionNameLabel

                    font: ui.theme.bodyBoldFont
                }
            }

            StyledTextLabel {
                anchors.horizontalCenter: parent.horizontalCenter

                text: qsTrc("midi", "Press a key or adjust a control on your MIDI device to\nassign it to this function.")
            }

            RowLayout {
                width: parent.width

                spacing: 10

                StyledTextLabel {
                    text: qsTrc("midi", "MIDI Mapping:")
                }

                TextInputField {
                    id: mappingField

                    Layout.fillWidth: true

                    readOnly: true

                    currentText: model.mappingTitle
                    hint: qsTrc("global", "Waiting...")
                }
            }

            Row {
                anchors.right: parent.right

                spacing: 8

                FlatButton {
                    width: 100

                    text: qsTrc("global", "Add")
                    enabled: mappingField.hasText

                    onClicked: {
                        root.mapToValueRequested(model.inputedValue())
                        root.close()
                    }
                }

                FlatButton {
                    width: 100

                    text: qsTrc("global", "Cancel")

                    onClicked: {
                        root.reject()
                    }
                }
            }
        }
    }
}
