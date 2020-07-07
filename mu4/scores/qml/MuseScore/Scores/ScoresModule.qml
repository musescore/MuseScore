import QtQuick 2.7
import MuseScore.UiComponents 1.0
import MuseScore.Scores 1.0

FocusScope {

    ScoresModel {
        id: scoresModel
    }

    Rectangle {
        anchors.fill: parent

        color: "#2C2C2C"
    }

    Text {
        anchors.bottom: scoresRect.top
        anchors.bottomMargin: 25
        anchors.left: scoresRect.left

        text: qsTrc("scores", "Scores")

        color: "#747474"
        font.family: ui.theme.font.family
        font.pixelSize: 30
    }

    Rectangle {
        id: scoresRect

        anchors.fill: parent
        anchors.topMargin: 100
        anchors.leftMargin: 50
        anchors.rightMargin: 50
        anchors.bottomMargin: 75

        color: "#262626"
        border.color: "#595959"
        border.width: 1
        radius: 15

        GridView {
            id: view

            anchors.fill: parent
            anchors.margins: 50

            model: scoresModel.recentList

            clip: true

            cellHeight: 200
            cellWidth: 160

            boundsBehavior: Flickable.StopAtBounds

            delegate: Item {
                height: view.cellHeight
                width: view.cellWidth

                ScoreItem {
                    property var score: modelData

                    anchors.centerIn: parent

                    height: 150
                    width: 100

                    title: score.title
                    thumbnail: score.thumbnail
                    isAdd: index === 0

                    onClicked: {
                        scoresModel.openRecentScore(index)
                    }
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
            text: qsTrc("scores", "Open a score")
            onClicked: scoresModel.openScore()
        }

        FlatButton {
            anchors.verticalCenter: parent ? parent.verticalCenter : undefined
            width: 100
            text: qsTrc("scores", "Import")
            onClicked: scoresModel.importScore()
        }
    }
}
