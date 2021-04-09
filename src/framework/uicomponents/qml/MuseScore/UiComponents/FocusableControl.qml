import QtQuick 2.15
import MuseScore.Ui 1.0

FocusScope {
    id: root

    default property alias content: contentItem.data
    property alias background: focusRectItem

    property alias mouseArea: mouseAreaItem
    property alias pressAndHoldInterval: mouseAreaItem.pressAndHoldInterval

    property alias keynav: keynavItem

    function insureActiveFocus() {
        if (!root.activeFocus) {
            root.forceActiveFocus()
        }

        if (!keynavItem.active) {
            keynavItem.forceActive()
        }
    }

    KeyNavigationControl {
        id: keynavItem
        name: root.objectName
        enabled: root.enabled

        onActiveChanged: {
            if (keynavItem.active) {
                root.insureActiveFocus()
            }
        }
    }

    Rectangle {
        id: focusRectItem
        anchors.fill: parent
        border.color: ui.theme.focusColor
        border.width: keynavItem.active ? 2 : 0
    }

    Item {
        id: contentItem
        anchors.fill: focusRectItem
        anchors.margins: 2 //! NOTE margin needed to show focus border
    }

    MouseArea {
        id: mouseAreaItem
        anchors.fill: parent

        onClicked: {
            root.insureActiveFocus()
        }
    }
}
