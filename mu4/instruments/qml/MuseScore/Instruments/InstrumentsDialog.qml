import QtQuick 2.9

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Instruments 1.0

QmlDialog {
    id: root

    property string canSelectMultipleInstruments: "true"

    height: 500
    width: root.canSelectMultipleInstruments === "true" ? 900 : 600

    title: qsTrc("instruments", "Instruments")

    Rectangle {
        anchors.fill: parent

        color: ui.theme.backgroundPrimaryColor

        ChooseInstrumentsPage {
            id: instrumentsPage

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: buttons.top
            anchors.bottomMargin: 10

            canSelectMultipleInstruments: root.canSelectMultipleInstruments === "true"
        }

        Row {
            id: buttons

            anchors.right: parent.right
            anchors.rightMargin: 10
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 10

            spacing: 8

            readonly property int buttonWidth: 134

            FlatButton {
                width: buttons.buttonWidth

                text: qsTrc("instruments", "Cancel")

                onClicked: {
                    root.ret = {errcode: 3}
                    root.hide()
                }
            }

            FlatButton {
                width: buttons.buttonWidth

                text: qsTrc("instruments", "Ok")

                onClicked: {
                    root.ret = {errcode: 0, value: instrumentsPage.selectedInstruments()}
                    root.hide()
                }
            }
        }
    }
}
