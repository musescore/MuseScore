import QtQuick 2.9
import QtQuick.Layouts 1.3
import MuseScore.Inspectors 3.3
import "../../common"

PopupViewButton {
    id: root

    property alias model: markerPopup.model

    icon: IconNameTypes.MARKER
    text: qsTr("Markers")

    visible: root.model ? !root.model.isEmpty : false

    MarkerPopup {
        id: markerPopup

        x: popupPositionX
        y: popupPositionY
        arrowX: parent.x + parent.width / 2
        width: popupAvailableWidth
    }
}
