import QtQuick 2.7
import QtQuick.Layouts 1.3
import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0

import "internal"

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

    Rectangle {
        anchors.fill: parent

        color: ui.theme.backgroundPrimaryColor
    }

    RowLayout {
        id: topLayout

        anchors.top: parent.top
        anchors.topMargin: 66
        anchors.left: parent.left
        anchors.leftMargin: privateProperties.sideMargin
        anchors.right: parent.right
        anchors.rightMargin: privateProperties.sideMargin

        spacing: 12

        StyledTextLabel {
            id: pageTitle

            text: qsTrc("userscores", "Scores")

            font.pixelSize: 32
            font.bold: true
        }

        SearchField {
            id: searchField

            Layout.maximumWidth: width
            Layout.alignment: Qt.AlignHCenter
        }

        Item {
            Layout.preferredWidth: pageTitle.width
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

        RecentScoresView {
            anchors.fill: parent

            model: FilterProxyModel {
                sourceModel: recentScoresModel

                filters: [
                    FilterValue {
                        roleName: "title"
                        roleValue: searchField.searchText
                        compareType: CompareType.Contains
                    }
                ]
            }

            onAddNewScoreRequested: {
                recentScoresModel.addNewScore()
            }

            onOpenScoreRequested: {
                recentScoresModel.openRecentScore(scorePath)
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
                text: qsTrc("userscores", "New")

                onClicked: {
                    recentScoresModel.addNewScore()
                }
            }

            FlatButton {
                width: privateProperties.buttonWidth
                text: qsTrc("userscores", "Open other...")

                onClicked: {
                    recentScoresModel.openScore()
                }
            }
        }
    }
}
