import QtQuick 2.7
import QtQuick.Layouts 1.15
import MuseScore.UiComponents 1.0
import MuseScore.Scores 1.0

FocusScope {
    id: root

    QtObject {
        id: privateProperties

        readonly property int sideMargin: 134
        readonly property int buttonWidth: 134
    }

    RecentScoresModel {
        id: recentScoresModel
    }

    FilterProxyModel {
        id: recentScoresFilterModel

        sourceModel: recentScoresModel

        filters: [
            FilterValue {
                roleName: "title"
                roleValue: searchField.currentText
                compareType: CompareType.Contains
            }
        ]
    }

    Rectangle {
        anchors.fill: parent

        color: ui.theme.backgroundColor
    }

    RowLayout {
        id: topLayout
        anchors.top: parent.top
        anchors.topMargin: 66
        anchors.left: parent.left
        anchors.right: parent.right

        spacing: 12

        StyledTextLabel {
            id: pageTitle

            Layout.leftMargin: privateProperties.sideMargin
            Layout.alignment: Qt.AlignLeft

            text: qsTrc("scores", "Scores")

            font.pixelSize: 32
            font.bold: true
        }

        SearchField {
            id: searchField

            Layout.alignment: Qt.AlignHCenter
        }

        Item {
            width: pageTitle.width
            Layout.rightMargin: privateProperties.sideMargin
            Layout.alignment: Qt.AlignLeft
        }
    }

    Item {
        anchors.top: topLayout.bottom
        anchors.topMargin: 74
        anchors.left: parent.left
        anchors.leftMargin: privateProperties.sideMargin
        anchors.right: parent.right
        anchors.rightMargin: privateProperties.sideMargin
        anchors.bottom: buttonsPanel.top

        Item {
            anchors.fill: parent
            anchors.leftMargin: -24
            anchors.rightMargin: -24

            Rectangle {
                anchors.top: parent.top

                width: parent.width
                height: 8
                z: 1

                gradient: Gradient {
                    GradientStop {
                        position: 0.0
                        color: ui.theme.backgroundColor
                    }

                    GradientStop {
                        position: 1.0
                        color: "transparent"
                    }
                }
            }

            GridView {
                id: recentScoresView

                anchors.fill: parent

                clip: true

                cellHeight: 334
                cellWidth: 220

                header: Item {
                    height: headerTitle.height
                    anchors.left: parent.left
                    anchors.right: parent.right

                    StyledTextLabel {
                        id: headerTitle

                        anchors.top: parent.top
                        anchors.topMargin: 8
                        anchors.left: parent.left
                        anchors.leftMargin: 24

                        text: qsTrc("scores", "New & recent")

                        font.pixelSize: 18
                        font.bold: true
                    }
                }

                model: recentScoresFilterModel

                delegate: Item {
                    height: recentScoresView.cellHeight
                    width: recentScoresView.cellWidth

                    ScoreItem {
                        anchors.centerIn: parent

                        height: 272
                        width: 172

                        title: score.title
                        thumbnail: score.thumbnail
                        isAdd: index === 0

                        onClicked: {
                            recentScoresModel.openRecentScore(index)
                        }
                    }
                }
            }

            Rectangle {
                anchors.bottom: parent.bottom

                width: parent.width
                height: 8
                z: 1

                gradient: Gradient {
                    GradientStop {
                        position: 0.0
                        color: "transparent"
                    }

                    GradientStop {
                        position: 1.0
                        color: ui.theme.backgroundColor
                    }
                }
            }
        }
    }

    Rectangle {
        id: buttonsPanel

        anchors.bottom: parent.bottom

        height: 114
        width: parent.width

        color: ui.theme.popupBackgroundColor

        Row {
            anchors.right : parent.right
            anchors.rightMargin: privateProperties.sideMargin
            anchors.verticalCenter: parent.verticalCenter

            spacing: 22

            FlatButton {
                width: privateProperties.buttonWidth
                text: qsTrc("scores", "New")

                onClicked: {
                    recentScoresModel.newScore()
                }
            }

            FlatButton {
                width: privateProperties.buttonWidth
                text: qsTrc("scores", "Open other...")

                onClicked: {
                    recentScoresModel.openScore()
                }
            }
        }
    }
}
