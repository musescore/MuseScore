import QtQuick 2.9
import QtQuick.Layouts 1.3
import MuseScore.Inspectors 3.3
import "../../common"

PopupViewButton {
    id: root

    property alias model: bendPopup.model

    icon: IconNameTypes.GUITAR_BEND
    text: qsTr("Guitar bends")

    visible: root.model ? !root.model.isEmpty : false

    BendPopup {
        id: bendPopup

        x: popupPositionX
        y: popupPositionY
        arrowX: parent.x + parent.width / 2
        width: popupAvailableWidth
    }
}
