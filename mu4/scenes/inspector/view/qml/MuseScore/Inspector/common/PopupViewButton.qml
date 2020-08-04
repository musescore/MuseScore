import QtQuick 2.9
import QtQuick.Layouts 1.3
import MuseScore.UiComponents 1.0

FlatButton {
    id: root

    default property StyledPopup popup
    property var popupPositionX
    property var popupPositionY: height
    property var popupAvailableWidth
    readonly property int popupContentHeight: popup.isOpened ? popup.implicitHeight + popup.arrowHeight : 0

    iconPixelSize: 16

    Layout.fillWidth: true
    Layout.minimumWidth: (popupAvailableWidth - 4) / 2

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
