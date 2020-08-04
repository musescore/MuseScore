import QtQuick 2.9
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../common"

PopupViewButton {
    id: root

    property alias model: textFramePopup.model

    icon: IconCode.TEXT_FRAME
    text: qsTr("Text frames")

    visible: root.model ? !root.model.isEmpty : false

    TextFramePopup {
        id: textFramePopup

        x: popupPositionX
        y: popupPositionY
        arrowX: parent.x + parent.width / 2
        width: popupAvailableWidth
    }
}
