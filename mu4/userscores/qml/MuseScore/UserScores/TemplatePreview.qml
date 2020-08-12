import QtQuick 2.7
import QtQuick.Layouts 1.3

import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0

Item {
    id: root

    StyledTextLabel {
        id: title

        anchors.top: parent.top

        text: qsTrc("userscores", "Preview")
        font.bold: true
    }

    Rectangle {
        anchors.top: title.bottom
        anchors.topMargin: 16
        anchors.bottom: parent.bottom

        width: parent.width

        color: "blue"
    }
}
