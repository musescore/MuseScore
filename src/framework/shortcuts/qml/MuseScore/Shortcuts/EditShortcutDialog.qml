import QtQuick 2.15
import QtQuick.Layouts 1.12

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

QmlDialog {
    id: root

    height: 223
    width: 538

    title: qsTrc("shortcuts", "Enter Shortcut Sequence")

    Rectangle {
        anchors.fill: parent

        color: ui.theme.backgroundPrimaryColor

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
                        Layout.fillWidth: true

                        hint: qsTrc("shortcuts", "Type to set shortcut")
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
                }

                Item { Layout.fillWidth: true }

                FlatButton {
                    width: parent.buttonWidth

                    text: qsTrc("global", "Add")
                }

                FlatButton {
                    width: parent.buttonWidth

                    text: qsTrc("global", "Replace")
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

