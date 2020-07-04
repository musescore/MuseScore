import QtQuick 2.7
import MuseScore.UiComponents 1.0
import MuseScore.Scores 1.0

FocusScope {

    ScoresModel {
        id: scoresModel
    }

    Rectangle {
        anchors.fill: parent
        anchors.topMargin: 75
        anchors.leftMargin: 50
        anchors.rightMargin: 50
        anchors.bottomMargin: 50

        color: "gray"
        border.color: "white"
        border.width: 2

        GridView {
            id: view

            anchors.fill: parent
            anchors.margins: 8

            model: scoresModel.recentList

            cellHeight: 160
            cellWidth: 110

            delegate: Column {
                property var score: modelData

                width: 100
                spacing: 10

                onScoreChanged: {
                    thumb.setThumbnail(score.thumbnail)
                }

                Rectangle {
                    color: score.title === "add" ? "red" : "blue"

                    height: 150
                    width: parent.width

                    ScoreThumbnail {
                        id: thumb

                        anchors.fill: parent

                        visible: score.title !== "add"
                    }
                }
                Text {
                    anchors.horizontalCenter: parent.horizontalCenter

                    text: score.title
                }
            }
        }

    }

    Row {
        anchors.left: parent.left
        anchors.leftMargin: 50
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 16

        spacing: 20

        FlatButton {
            anchors.verticalCenter: parent ? parent.verticalCenter : undefined
            width: 100
            text: "Open score"
            onClicked: scoresModel.openScore()
        }

        FlatButton {
            anchors.verticalCenter: parent ? parent.verticalCenter : undefined
            width: 100
            text: "Import score"
            onClicked: scoresModel.importScore()
        }
    }
}
