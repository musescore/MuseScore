import QtQuick 2.9
import QtQuick.Layouts 1.3
import MuseScore.Inspectors 3.3
import "../../common"

PopupViewButton {
    id: root

    property alias model: crescendoPopup.model

    icon: IconNameTypes.CRESCENDO_LINE
    text: qsTr("Crescendo")

    visible: root.model ? !root.model.isEmpty : false

    CrescendoPopup {
        id: crescendoPopup

        x: popupPositionX
        y: popupPositionY
        arrowX: parent.x + parent.width / 2
        width: popupAvailableWidth
    }
}
