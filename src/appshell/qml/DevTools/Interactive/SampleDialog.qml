import QtQuick 2.7
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

QmlDialog {

    id: root

    property var color: "#444444"
    property bool isApplyColor: false

    width: 400
    height: 400

    title: "Sample dialog"

    Rectangle {

        anchors.fill: parent
        color: root.isApplyColor ? root.color : "#666666"

        Column {
            anchors.centerIn: parent

            spacing: 50

            TextInputField {
                id: input
                anchors.horizontalCenter: parent.horizontalCenter

                property var value: ""
                width: 150
                onCurrentTextEdited: input.value = newTextValue
            }

            StyledTextLabel {
                anchors.horizontalCenter: parent.horizontalCenter

                text: "Use right click for showing context menu"
            }
        }

        Row {
            anchors.bottom: parent.bottom
            anchors.right: parent.right

            anchors.rightMargin: 16
            anchors.bottomMargin: 20
            spacing: 20

            FlatButton {
                text: "Cancel"
                onClicked: {
                    root.reject()
                }
            }

            FlatButton {
                text: "OK"
                onClicked: {
                    root.ret = {errcode: 0, value: input.value }
                    root.hide()
                }
            }
        }

        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.RightButton
            onClicked: menu.popup()
        }

        ContextMenu {
            id: menu

            StyledContextMenuItem {
                hintIcon: IconCode.UNDO

                text: "Undo"
                shortcut: "Ctrl+Z"
            }

            StyledContextMenuItem {
                hintIcon: IconCode.REDO

                text: "Redo"
                shortcut: "Shift+Ctrl+Z"

                enabled: false
            }

            SeparatorLine {}

            StyledContextMenuItem {
                hintIcon: IconCode.ZOOM_IN

                text: "Zoom in"
            }

            StyledContextMenuItem {
                hintIcon: IconCode.ZOOM_OUT

                text: "Zoom out"
            }

            SeparatorLine {}

            StyledContextMenuItem {
                text: "Checkable"

                checkable: true
                checked: false
            }
        }
    }
}
