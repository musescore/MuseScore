import QtQuick 2.15
import MuseScore.Ui 1.0

FocusScope {
    id: root

    default property alias content: contentItem.data
    property alias focusRect: focusRectItem

    property alias mouseArea: mouseAreaItem
    property alias pressAndHoldInterval: mouseAreaItem.pressAndHoldInterval

    property alias keynav: keynavItem

    signal internalPressed()
    signal internalReleased()
    signal internalClicked()
    signal internalPressAndHold()

    signal internalTriggered()

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

        onActiveChanged: {
            if (keynavItem.active) {
                root.insureActiveFocus()
            }
        }

        onTriggered: {
            root.internalTriggered()
        }
    }

    Rectangle {
        id: focusRectItem
        anchors.fill: parent

        Item {
            id: contentItem
            anchors.fill: parent
            anchors.margins: 2
        }

        border.color: ui.theme.focusColor
        border.width: root.focus ? 2 : 0
    }

    MouseArea {
        id: mouseAreaItem
        anchors.fill: parent

        onClicked: {
            root.insureActiveFocus()
            root.internalClicked()
        }

        onPressAndHold: {
            root.internalPressAndHold()
        }

        onPressed: {
            root.internalPressed()
        }

        onReleased: {
            root.internalReleased()
        }
    }
}
