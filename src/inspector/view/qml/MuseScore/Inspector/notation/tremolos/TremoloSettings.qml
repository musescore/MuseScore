import QtQuick 2.9
import QtQuick.Layouts 1.3
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../common"

PopupViewButton {
    id: root

    property alias model: tremoloPopup.model

    icon: IconCode.TREMOLO_TWO_NOTES
    text: qsTr("Tremolos")

    visible: root.model ? !root.model.isEmpty : false

    TremoloPopup {
        id: tremoloPopup

        x: popupPositionX
        y: popupPositionY
        arrowX: parent.x + parent.width / 2
        width: popupAvailableWidth
    }
}
