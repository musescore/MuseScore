import QtQuick 2.9
import MuseScore.Inspectors 3.3

import "../../common"

PopupViewButton {
    id: root

    property alias model: bracketPopup.model

    icon: IconNameTypes.BRACE
    text: qsTr("Braces")

    visible: root.model ? !root.model.isEmpty : false

    BracketPopup {
        id: bracketPopup

        x: popupPositionX
        y: popupPositionY
        arrowX: parent.x + parent.width / 2
        width: popupAvailableWidth
    }
}
