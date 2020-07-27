import QtQuick 2.7
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0

QmlDialog {
    id: root

    height: 300
    width: 700

    Rectangle {

        NewScoreModel {
            id: newScoreModel
        }

        anchors.fill: parent
        color: ui.theme.backgroundColor

        Column {
            anchors.centerIn: parent

            width: parent.width
            height: 100
            spacing: 20

            Row {
                spacing: 20

                anchors.horizontalCenter: parent.horizontalCenter

                StyledTextLabel {
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTrc("userscores", "Score title:")
                }

                TextInputField {
                    id: scoreTitle

                    height: 40
                    width: 100

                    onCurrentTextEdited: {
                        newScoreModel.title = newTextValue
                    }

                }

                StyledTextLabel {
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTrc("userscores", "composer:")
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

                anchors.horizontalCenter: parent.horizontalCenter

                FlatButton {

                    width: 100
                    text: qsTrc("userscores", "Ok")
                    onClicked: {
                        var ret = newScoreModel.create()
                        if (ret) {
                            root.ret = {errcode: 0}
                            root.hide()
                        }
                    }
                }

                FlatButton {
                    width: 100
                    text: qsTrc("userscores", "Cancel")
                    onClicked: {
                        root.ret = {errcode: 3}
                        root.hide()
                    }
                }
            }
        }
    }
}
