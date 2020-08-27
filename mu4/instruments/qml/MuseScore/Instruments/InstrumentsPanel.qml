import QtQuick 2.9

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Instruments 1.0

Item {
    Rectangle {
        anchors.fill: parent

        color: ui.theme.backgroundPrimaryColor
    }

    Row {
        id: buttons

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: 8

        spacing: 4

        FlatButton {
            width: 134
            text: qsTrc("instruments", "Add")

            onClicked: {

            }
        }

        FlatButton {
            width: 30
            icon: IconCode.ARROW_UP

            onClicked: {

            }
        }

        FlatButton {
            width: 30
            icon: IconCode.ARROW_DOWN

            onClicked: {

            }
        }

        FlatButton {
            width: 30
            icon: IconCode.TRASH

            onClicked: {

            }
        }
    }

    Rectangle {
        anchors.top: buttons.bottom
        anchors.topMargin: 16
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        color: "#3698da"
    }
}
