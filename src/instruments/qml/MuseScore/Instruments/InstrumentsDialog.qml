import QtQuick 2.9

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Instruments 1.0

QmlDialog {
    id: root

    property bool canSelectMultipleInstruments: true
    property string currentInstrumentId: ""
    property string initiallySelectedInstrumentIds: ""

    height: 500
    width: root.canSelectMultipleInstruments ? 900 : 600

    title: canSelectMultipleInstruments ? qsTrc("instruments", "Instruments") :
                                          qsTrc("instruments", "Select instrument")

    Rectangle {
        anchors.fill: parent

        color: ui.theme.backgroundPrimaryColor

        ChooseInstrumentsPage {
            id: instrumentsPage

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: buttons.top
            anchors.margins: 10

            initiallySelectedInstrumentIds: root.initiallySelectedInstrumentIds
            canSelectMultipleInstruments: root.canSelectMultipleInstruments
            currentInstrumentId: root.currentInstrumentId
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

                text: qsTrc("global", "Cancel")

                onClicked: {
                    root.reject()
                }
            }

            FlatButton {
                width: buttons.buttonWidth

                text: qsTrc("global", "OK")

                onClicked: {
                    var selectedInstruments = instrumentsPage.selectedInstruments()

                    root.ret = { errcode: 0, value: selectedInstruments }
                    root.hide()
                }
            }
        }
    }
}
