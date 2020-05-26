import QtQuick 2.9
import QtQuick.Layouts 1.3
import MuseScore.Inspectors 3.3
import "../../common"

PopupViewButton {
    id: root

    property alias model: chordSymbolPopup.model

    icon: IconNameTypes.CHORD_SYMBOL
    text: qsTr("Chord symbols")

    visible: root.model ? !root.model.isEmpty : false

    ChordSymbolPopup {
        id: chordSymbolPopup

        x: popupPositionX
        y: popupPositionY
        arrowX: parent.x + parent.width / 2
        width: popupAvailableWidth
    }
}
