import QtQuick 2.9
import QtQuick.Layouts 1.3
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../common"

PopupViewButton {
    id: root

    property alias model: chordSymbolPopup.model

    icon: IconCode.CHORD_SYMBOL
    text: qsTrc("inspector", "Chord symbols")

    visible: root.model ? !root.model.isEmpty : false

    ChordSymbolPopup {
        id: chordSymbolPopup

        x: popupPositionX
        y: popupPositionY
        arrowX: parent.x + parent.width / 2
        width: popupAvailableWidth
    }
}
