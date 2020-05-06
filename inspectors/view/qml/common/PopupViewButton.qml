import QtQuick 2.9
import QtQuick.Layouts 1.3

FlatButton {
    id: root

    default property StyledPopup popup
    property var popupPositionX
    property var popupPositionY: height
    property var popupAvailableWidth

    iconPixelSize: 16

    Layout.fillWidth: true
    Layout.minimumWidth: popupAvailableWidth / 2

    onVisibleChanged: {
        if (!visible) {
            popup.close()
        }
    }

    onClicked: {
        if (!popup.isOpened) {
            popup.open()
        } else {
            popup.close()
        }
    }
}
