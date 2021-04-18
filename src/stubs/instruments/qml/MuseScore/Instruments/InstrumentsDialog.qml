import QtQuick 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

QmlDialog {
    id: root

    property bool canSelectMultipleInstruments: false
    property string currentInstrumentId: ""
    property string initiallySelectedInstrumentIds: ""

    height: 500
    width: 900

    title: canSelectMultipleInstruments ? qsTrc("instruments", "Instruments") :
                                          qsTrc("instruments", "Select instrument")

    Rectangle {
        anchors.fill: parent

        color: ui.theme.backgroundPrimaryColor

        StyledTextLabel {
            anchors.centerIn: parent
            text: "Instrumens Dialog Stub"
        }
    }
}
