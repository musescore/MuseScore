import QtQuick 2.7

import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Row {
    spacing: 4

    ConcertPitchControlModel {
        id: model

        function toggleConcertPitch() {
            model.concertPitchEnabled = !model.concertPitchEnabled
        }
    }

    Component.onCompleted: {
        model.load()
    }

    CheckBox {
        anchors.verticalCenter: parent.verticalCenter

        checked: model.concertPitchEnabled

        onClicked: {
            model.toggleConcertPitch()
        }
    }

    FlatButton {
        icon: IconCode.TUNING_FORK
        text: qsTrc("notation", "Concert pitch")

        orientation: Qt.Horizontal
        normalStateColor: "transparent"

        onClicked: {
            model.toggleConcertPitch()
        }
    }
}
