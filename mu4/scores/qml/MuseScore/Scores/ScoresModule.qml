import QtQuick 2.7
import MuseScore.UiComponents 1.0
import MuseScore.Scores 1.0

FocusScope {

    ScoresModel {
        id: scoresModel
    }

    FlatButton {
        anchors.centerIn: parent
        width: 100
        text: "Open score"
        onClicked: scoresModel.openScore()
    }
}
