import QtQuick 2.9
import QtQuick.Layouts 1.3
import MuseScore.Inspectors 3.3
import "../../common"

PopupViewButton {
    id: root

    property alias model: accidentalPopup.model

    icon: IconNameTypes.ACCIDENTAL_SHARP
    text: qsTr("Accidentals")

    visible: root.model ? !root.model.isEmpty : false

    AccidentalPopup {
        id: accidentalPopup

        x: popupPositionX
        y: popupPositionY
        arrowX: parent.x + parent.width / 2
        width: popupAvailableWidth
    }
}
