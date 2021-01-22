import QtQuick 2.15

import MuseScore.UiComponents 1.0

Rectangle {
    id: root

    property string initiallySelectedInstrumentIds: ""
    property bool hasSelectedInstruments: false
    property bool canSelectMultipleInstruments: false
    property string currentInstrumentId: ""

    function selectedInstruments() {
        return []
    }

    color: ui.theme.backgroundPrimaryColor

    function focusOnCurrentInstrument() {
    }

    StyledTextLabel {
        anchors.centerIn: parent
        text: "Choose Instruments Page Stub"
    }
}
