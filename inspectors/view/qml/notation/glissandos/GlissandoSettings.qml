import QtQuick 2.9
import QtQuick.Layouts 1.3
import MuseScore.Inspectors 3.3
import "../../common"

PopupViewButton {
    id: root

    property alias model: glissandoPopup.model

    icon: IconNameTypes.GLISSANDO
    text: qsTr("Glissandos")

    visible: root.model ? !root.model.isEmpty : false

    GlissandoPopup {
        id: glissandoPopup

        x: popupPositionX
        y: popupPositionY
        arrowX: parent.x + parent.width / 2
        width: popupAvailableWidth
    }
}
