import QtQuick 2.9
import QtQuick.Layouts 1.3
import MuseScore.Inspectors 3.3
import "../../common"

PopupViewButton {
    id: root

    property alias model: imagePopup.model

    icon: IconNameTypes.IMAGE_MOUNTAINS
    text: qsTr("Images")

    visible: root.model ? !root.model.isEmpty : false

    ImagePopup {
        id: imagePopup

        x: popupPositionX
        y: popupPositionY
        arrowX: parent.x + parent.width / 2
        width: popupAvailableWidth
    }
}
