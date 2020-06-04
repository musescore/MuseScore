import QtQuick 2.9
import QtQuick.Layouts 1.3
import MuseScore.Inspectors 3.3
import "../../common"

PopupViewButton {
    id: root

    property alias model: ambitusPopup.model

    icon: IconNameTypes.AMBITUS
    text: qsTr("Ambitus")

    visible: root.model ? !root.model.isEmpty : false

    AmbitusPopup {
        id: ambitusPopup

        x: popupPositionX
        y: popupPositionY
        arrowX: parent.x + parent.width / 2
        width: popupAvailableWidth
    }
}
