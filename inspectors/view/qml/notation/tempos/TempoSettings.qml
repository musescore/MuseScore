import QtQuick 2.9
import QtQuick.Layouts 1.3
import MuseScore.Inspectors 3.3
import "../../common"

PopupViewButton {
    id: root

    property alias model: tempoPopup.model

    icon: IconNameTypes.METRONOME
    text: qsTr("Tempos")

    visible: root.model ? !root.model.isEmpty : false

    TempoPopup {
        id: tempoPopup

        x: popupPositionX
        y: popupPositionY
        arrowX: parent.x + parent.width / 2
        width: popupAvailableWidth
    }
}
