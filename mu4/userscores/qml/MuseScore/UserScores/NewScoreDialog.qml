import QtQuick 2.9
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0

QmlDialog {
    id: root

    height: 500
    width: 800

    Rectangle {

        NewScoreModel {
            id: newScoreModel
        }

        anchors.fill: parent
        color: ui.theme.backgroundColor

        StackLayout {
            id: pagesStack
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: buttons.top
            anchors.bottomMargin: 10

            Rectangle {
                anchors.fill: parent

                color: "green"
            }

            Rectangle {
                anchors.fill: parent

                color: "red"
            }
        }

        Row {
            id: buttons
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10

            spacing: 5

            FlatButton {
                text: qsTrc("userscores", "Cancel")
            }
            FlatButton {
                visible: pagesStack.currentIndex > 0

                text: qsTrc("userscores", "Back")

                onClicked: {
                    pagesStack.currentIndex--
                }
            }
            FlatButton {
                visible: pagesStack.currentIndex < pagesStack.count - 1

                text: qsTrc("userscores", "Next")

                onClicked: {
                    pagesStack.currentIndex++
                }
            }
            FlatButton {
                text: qsTrc("userscores", "Done")
            }
        }
    }
}
