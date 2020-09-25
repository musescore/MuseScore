import QtQuick 2.7
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

QmlDialog {

    id: root

    property var color: "#444444"

    width: 400
    height: 400

    Rectangle {

        anchors.fill: parent
        color: root.color

        TextInputField {
            id: input
            property var value: ""
            anchors.centerIn: parent
            width: 150
            height: 32
            onCurrentTextEdited: input.value = newTextValue
        }

        Row {
            anchors.bottom: parent.bottom
            anchors.right: parent.right
            anchors.rightMargin: 16
            height: 40
            width:  100
            spacing: 20

            FlatButton {
                text: "Ok"
                onClicked: {
                    root.ret = {errcode: 0, value: input.value }
                    root.hide()
                }
            }

            FlatButton {
                text: "Cancel"
                onClicked: {
                    root.reject()
                }
            }
        }
    }
}
