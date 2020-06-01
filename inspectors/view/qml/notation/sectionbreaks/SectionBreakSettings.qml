import QtQuick 2.9
import QtQuick.Layouts 1.3
import MuseScore.Inspectors 3.3
import "../../common"

PopupViewButton {
    id: root

    property alias model: sectionBreakPopup.model

    icon: IconNameTypes.SECTION_BREAK
    text: qsTr("Section breaks")

    visible: root.model ? !root.model.isEmpty : false

    SectionBreakPopup {
        id: sectionBreakPopup

        x: popupPositionX
        y: popupPositionY
        arrowX: parent.x + parent.width / 2
        width: popupAvailableWidth
    }
}
