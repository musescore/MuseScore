import QtQuick 2.9
import QtQuick.Layouts 1.3
import MuseScore.Inspector 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../common"

PopupViewButton {
    id: root

    property alias model: glissandoPopup.model

    icon: IconCode.GLISSANDO
    text: qsTrc("inspector", "Glissandos")

    visible: root.model ? !root.model.isEmpty : false

    GlissandoPopup {
        id: glissandoPopup

        x: popupPositionX
        y: popupPositionY
        arrowX: parent.x + parent.width / 2
        width: popupAvailableWidth
    }
}
