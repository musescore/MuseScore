import QtQuick 2.9
import QtQuick.Layouts 1.3
import MuseScore.Inspectors 3.3
import MuseScore.UiComponents 1.0
import "../common"

PopupViewButton {
    id: root

    property alias model: barPopup.model

    text: qsTr("Insert bars")

    MeasurePopup {
        id: barPopup

        x: popupPositionX
        y: popupPositionY
        arrowX: parent.x + parent.width / 2
        width: popupAvailableWidth
    }
}
