import QtQuick 2.7
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Item {
    id: root

    Column {
        anchors.fill: parent
        anchors.margins: 20

        spacing: 20

        FlatButton {
            width: 220

            text: qsTrc("devtools", "Measure properties")

            onClicked: {
                api.launcher.open("musescore://notation/measureproperties?index=0")
            }
        }

        FlatButton {
            width: 220

            text: qsTrc("devtools", "Add/Remove System Breaks")

            onClicked: {
                api.launcher.open("musescore://notation/breaks")
            }
        }

        FlatButton {
            width: 220

            text: qsTrc("devtools", "Score properties")

            onClicked: {
                api.launcher.open("musescore://notation/properties")
            }
        }

        FlatButton {
            width: 220

            text: qsTrc("devtools", "Style dialog")

            onClicked: {
                api.launcher.open("musescore://notation/style")
            }
        }
    }
}
