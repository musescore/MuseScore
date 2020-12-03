import QtQuick 2.9
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../common"

PopupViewButton {
    id: root

    property alias model: verticalFramePopup.model

    icon: IconCode.VERTICAL_FRAME
    text: qsTr("Vertical frames")

    visible: root.model ? !root.model.isEmpty : false

    VerticalFramePopup {
        id: verticalFramePopup

        x: popupPositionX
        y: popupPositionY
        arrowX: parent.x + parent.width / 2
        width: popupAvailableWidth
    }
}
