import QtQuick 2.9
import QtQuick.Layouts 1.3
import MuseScore.Inspectors 3.3
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import "../../common"

PopupViewButton {
    id: root

    property alias model: articulationPopup.model

    icon: IconCode.ARTICULATION
    text: qsTr("Articulations")

    visible: root.model ? !root.model.isEmpty : false

    ArticulationPopup {
        id: articulationPopup

        x: popupPositionX
        y: popupPositionY
        arrowX: parent.x + parent.width / 2
        width: popupAvailableWidth
    }
}
