import QtQuick 2.7
import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0

Item {
    id: root

    property alias model: recentScoresView.model

    signal openScoreRequested(var scorePath)
    signal addNewScoreRequested()

    QtObject {
        id: privateProperties

        readonly property int shadowHeight: 8
    }

    Rectangle {
        anchors.top: parent.top

        width: parent.width
        height: privateProperties.shadowHeight
        z: 1

        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: ui.theme.backgroundPrimaryColor
            }

            GradientStop {
                position: 1.0
                color: "transparent"
            }
        }
    }

    GridView {
        id: recentScoresView

        readonly property int sideMargin: 34

        anchors.fill: parent
        anchors.leftMargin: -sideMargin
        anchors.rightMargin: -sideMargin

        clip: true

        cellHeight: 334
        cellWidth: sideMargin + 172 + sideMargin

        header: Item {
            height: headerTitle.height
            anchors.left: parent.left
            anchors.right: parent.right

            StyledTextLabel {
                id: headerTitle

                anchors.top: parent.top
                anchors.topMargin: privateProperties.shadowHeight
                anchors.left: parent.left
                anchors.leftMargin: recentScoresView.sideMargin

                text: qsTrc("userscores", "New & recent")

                font.pixelSize: 18
                font.bold: true
            }
        }

        delegate: Item {
            height: recentScoresView.cellHeight
            width: recentScoresView.cellWidth

            ScoreItem {
                anchors.centerIn: parent

                height: 272
                width: 172

                title: score.title
                thumbnail: score.thumbnail
                isAdd: score.isAddNew
                timeSinceCreation: !isAdd ? score.timeSinceCreation : ""

                onClicked: {
                    if (isAdd) {
                        root.addNewScoreRequested()
                    } else {
                        root.openScoreRequested(score.path)
                    }
                }
            }
        }
    }

    Rectangle {
        anchors.bottom: parent.bottom

        width: parent.width
        height: privateProperties.shadowHeight
        z: 1

        gradient: Gradient {
            GradientStop {
                position: 0.0
                color: "transparent"
            }

            GradientStop {
                position: 1.0
                color: ui.theme.backgroundPrimaryColor
            }
        }
    }
}
