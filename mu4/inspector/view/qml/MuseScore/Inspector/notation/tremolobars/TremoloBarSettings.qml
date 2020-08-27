import QtQuick 2.9
import QtQuick.Layouts 1.3
import MuseScore.Inspector 1.0
import MuseScore.Ui 1.0
import "../../common"

PopupViewButton {
    id: root

    property alias model: tremoloBarPopup.model

    icon: IconCode.GUITAR_TREMOLO_BAR
    text: qsTr("Guitar tremolo bars")

    visible: root.model ? !root.model.isEmpty : false

    TremoloBarPopup {
        id: tremoloBarPopup

        x: popupPositionX
        y: popupPositionY
        arrowX: parent.x + parent.width / 2
        width: popupAvailableWidth
    }
}
