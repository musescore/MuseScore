import QtQuick 2.9
import QtQuick.Layouts 1.3
import MuseScore.Inspectors 3.3
import "../../common"

PopupViewButton {
    id: root

    property alias model: notePopup.model

    popupAvailableWidth: parent ? parent.width : 0

    icon: IconNameTypes.MUSIC_NOTES
    text: qsTr("Note settings")

    visible: root.model ? !root.model.isEmpty : false

    NotePopup {
        id: notePopup

        x: popupPositionX
        y: popupPositionY
        arrowX: parent.x + parent.width / 2
        width: popupAvailableWidth
    }
}

