import QtQuick 2.9
import MuseScore.Inspectors 3.3
import "../../common"

PopupViewButton {
    id: root

    property alias model: verticalFramePopup.model

    icon: IconNameTypes.VERTICAL_FRAME
    text: qsTr("Vertical frame")

    visible: root.model ? !root.model.isEmpty : false

    VerticalFramePopup {
        id: verticalFramePopup

        x: popupPositionX
        y: popupPositionY
        arrowX: parent.x + parent.width / 2
        width: popupAvailableWidth
    }
}
