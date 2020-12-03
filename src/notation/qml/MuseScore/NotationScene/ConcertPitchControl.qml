import QtQuick 2.7
import MuseScore.NotationScene 1.0

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Row {
    spacing: 4

    ConcertPitchControlModel {
        id: model
    }

    Component.onCompleted: {
        model.load()
    }

    CheckBox {
        checked: model.concertPitchEnabled

        width: 20

        onClicked: {
            model.concertPitchEnabled = !checked
        }
    }

    StyledIconLabel {
        width: 32

        iconCode: IconCode.TUNING_FORK
    }

    StyledTextLabel {
        height: parent.height

        text: qsTrc("notation", "Concert pitch")
    }
}
