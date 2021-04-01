import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Shortcuts 1.0

Dialog {
    id: root

    signal applySequenceRequested(var newSequence)

    function startEdit(sequence, allShortcuts) {
        open()
        model.load(sequence, allShortcuts)
        newSequenceField.forceActiveFocus()
    }

    height: 240
    width: 538

    title: qsTrc("shortcuts", "Enter Shortcut Sequence")

    standardButtons: Dialog.NoButton

    EditShortcutModel {
        id: model
    }

    Rectangle {
        anchors.fill: parent

        color: ui.theme.backgroundPrimaryColor

        Column {
            anchors.fill: parent
            anchors.margins: 8

            spacing: 20

            StyledTextLabel {
                anchors.horizontalCenter: parent.horizontalCenter

                width: parent.width
                text: qsTrc("shortcuts", "Press up to four key combinations to enter shortcut sequence. \nNote: “Ctrl+Shift+1” is one key combination.")
            }

            Column {
                width: parent.width

                spacing: 12

                StyledTextLabel {
                    width: parent.width
                    horizontalAlignment: Qt.AlignLeft

                    text: model.errorMessage
                }

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
                            model.inputKey(event.key, event.modifiers)
                        }

                        onActiveFocusChanged: {
                            if (!activeFocus) {
                                forceActiveFocus()
                            }
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
                        model.clear()
                    }
                }

                Item { Layout.fillWidth: true }

                FlatButton {
                    width: parent.buttonWidth

                    text: qsTrc("global", "Add")
                    enabled: model.canApplySequence

                    onClicked: {
                        root.applySequenceRequested(model.unitedSequence())
                        root.accept()
                    }
                }

                FlatButton {
                    width: parent.buttonWidth

                    text: qsTrc("global", "Replace")
                    enabled: model.canApplySequence

                    onClicked: {
                        root.applySequenceRequested(model.inputedSequence)
                        root.accept()
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

