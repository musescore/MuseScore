import QtQuick 2.7
import MuseScore.UiComponents 1.0
import MuseScore.Scores 1.0

FocusScope {

    ScoresModel {
        id: scoresModel
    }

    Row {

        height: 100
        width: 220
        spacing: 20
        anchors.centerIn: parent

        FlatButton {
            anchors.verticalCenter: parent.verticalCenter
            width: 100
            text: "Open score"
            onClicked: scoresModel.openScore()
        }

        FlatButton {
            anchors.verticalCenter: parent.verticalCenter
            width: 100
            text: "Import score"
            onClicked: scoresModel.importScore()
        }
    }
}
