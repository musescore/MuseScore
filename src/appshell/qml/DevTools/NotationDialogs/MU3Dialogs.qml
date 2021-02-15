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

            text: "Measure properties"

            onClicked: {
                api.launcher.open("musescore://notation/measureproperties?index=0")
            }
        }

        FlatButton {
            width: 220

            text: "Add/Remove System Breaks"

            onClicked: {
                api.launcher.open("musescore://notation/breaks")
            }
        }

        FlatButton {
            width: 220

            text: "Score properties"

            onClicked: {
                api.launcher.open("musescore://notation/properties")
            }
        }

        FlatButton {
            width: 220

            text: "Style dialog"

            onClicked: {
                api.launcher.open("musescore://notation/style")
            }
        }

        FlatButton {
            width: 220

            text: "Transpose dialog"

            onClicked: {
                api.launcher.open("musescore://notation/transpose")
            }
        }

        FlatButton {
            width: 220

            text: "Staff/Part Properties"

            onClicked: {
                api.launcher.open("musescore://notation/staffproperties?staffIdx=0")
            }
        }

        FlatButton {
            width: 220

            text: "Master palette"

            onClicked: {
                //! NOTE: it is important to launch master palette with sync = false
                // for an ability to use drag & drop from this dialog
                api.launcher.open("musescore://palette/masterpalette?sync=false")
            }
        }
    }
}
