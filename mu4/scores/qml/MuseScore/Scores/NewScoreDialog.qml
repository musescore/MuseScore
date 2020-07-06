import QtQuick 2.7
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Scores 1.0

QmlDialog {

    id: root

    width: 400
    height: 400

    Rectangle {

        NewScoreModel {
            id: newScoreModel

            onClose: {
                root.ret = {errcode: 0}
                root.hide()
            }
        }

        anchors.fill: parent
        color: "#474747"

        Column {

            anchors.centerIn: parent

            width: 220
            height: 100
            spacing: 20

            Row {
                spacing: 20

                TextInputField {
                    id: scoreTitle

                    height: 40
                    width: 100

                    onCurrentTextEdited: {
                        newScoreModel.title = newTextValue
                    }
                }
                TextInputField {
                    id: scoreComposer

                    height: 40
                    width: 100

                    onCurrentTextEdited: {
                        newScoreModel.composer = newTextValue
                    }
                }
            }

            Row {
                height: 40
                width:  100
                spacing: 20

                FlatButton {

                    width: 40
                    text: "Yes"
                    onClicked: {
                        newScoreModel.create()
                    }
                }

                FlatButton {
                    width: 40
                    text: "Cancel"
                    onClicked: {
                        root.ret = {errcode: 0}
                        root.hide()
                    }
                }
            }
        }
    }
}
