import QtQuick 2.9
import QtQuick.Layouts 1.3
import MuseScore.Inspectors 3.3
import "../../common"

PopupViewButton {
    id: root

    property alias model: fretDiagramPopup.model

    icon: IconNameTypes.FRET_DIAGRAM
    text: qsTr("Fretboard Diagrams")

    visible: root.model ? !root.model.isEmpty : false

    FretDiagramPopup {
        id: fretDiagramPopup

        x: popupPositionX
        y: popupPositionY
        arrowX: parent.x + parent.width / 2
        width: popupAvailableWidth
    }
}
