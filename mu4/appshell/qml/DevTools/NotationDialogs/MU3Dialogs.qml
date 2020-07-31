import QtQuick 2.7
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Item {
    id: root

    FlatButton {
        width: 140

        text: qsTrc("devtools", "Measure properties")

        onClicked: {
            api.launcher.open("musescore://notation/measureproperties?index=0")
        }
    }
}
