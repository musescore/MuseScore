import QtQuick 2.15
import QtQuick.Layouts 1.12

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Shortcuts 1.0

QmlDialog {
    id: root

    height: 223
    width: 538

    title: qsTrc("shortcuts", "Enter Shortcut Sequence")

    property string sequence: ""

    Rectangle {
        anchors.fill: parent

        color: ui.theme.backgroundPrimaryColor

        EditShortcutModel {
            id: model
        }

        Component.onCompleted: {
            model.load(root.sequence)
        }

        Column {
            spacing: 30

            anchors.fill: parent
            anchors.margins: 16

            StyledTextLabel {
                anchors.horizontalCenter: parent.horizontalCenter

                width: parent.width
                text: qsTrc("shortcuts", "Press up to four key combinations to enter shortcut sequence. \nNote: “Ctrl+Shift+1” is one key combination.")
            }

            Column {
                width: parent.width

                spacing: 12

                RowLayout {
                    width: parent.width
                    height: childrenRect.height

                    spacing: 12

                    StyledTextLabel {
                        Layout.alignment: Qt.AlignVCenter

                        text: qsTrc("shortcuts", "Old shortcuts:")
                    }

                    TextInputField {
                        Layout.fillWidth: true

                        enabled: false
                        currentText: model.originSequence
                    }
                }

                RowLayout {
                    width: parent.width
                    height: childrenRect.height

                    spacing: 12

                    StyledTextLabel {
                        Layout.alignment: Qt.AlignVCenter

                        text: qsTrc("shortcuts", "New shortcut:")
                    }

                    TextInputField {
                        id: newSequenceField

                        Layout.fillWidth: true

                        hint: qsTrc("shortcuts", "Type to set shortcut")
                        readOnly: true
                        currentText: model.inputedSequence

                        Keys.onPressed: {
                            model.handleKey(event.key, event.modifiers)
                        }
                    }
                }
            }

            RowLayout {
                width: parent.width
                height: childrenRect.height

                readonly property int buttonWidth: 100

                FlatButton {
                    width: parent.buttonWidth

                    text: qsTrc("global", "Clear")

                    onClicked: {
                        newSequenceField.clear()
                    }
                }

                Item { Layout.fillWidth: true }

                FlatButton {
                    width: parent.buttonWidth

                    text: qsTrc("global", "Add")
                    enabled: newSequenceField.hasText

                    onClicked: {
                        root.ret = { errcode: 0, value: model.unitedSequence() }
                        root.hide()
                    }
                }

                FlatButton {
                    width: parent.buttonWidth

                    text: qsTrc("global", "Replace")
                    enabled: newSequenceField.hasText

                    onClicked: {
                        root.ret = { errcode: 0, value: model.inputedSequence }
                        root.hide()
                    }
                }

                FlatButton {
                    width: parent.buttonWidth

                    text: qsTrc("global", "Cancel")

                    onClicked: {
                        root.reject()
                    }
                }
            }
        }
    }
}

