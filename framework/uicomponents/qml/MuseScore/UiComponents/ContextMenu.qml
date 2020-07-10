import QtQuick 2.9
import QtQuick.Controls 2.2
import QtGraphicalEffects 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0

Rectangle {
    id: root

    property bool opened: false
    property bool hovered: false

    property alias icon: buttonIcon.iconCode

    function addMenuItem(title, itemAction) {
        contextMenu.addItem(menuItemComponent.createObject(root, { text : title, actionItem : itemAction }))
    }

    function clearMenuItems() {
        var o = 0

        for (var i = contextMenu.contentData.length - 1; i >= 0; --i) {
            contextMenu.removeItem(i)
        }
    }

    function forceOpen() {
        contextMenu.open()
    }

    function forceClose() {
        contextMenu.close()
    }

    height: 24
    width: 24

    radius: 4

    color: "transparent"

    opacity: root.hovered || root.opened

    StyledIconLabel {
        id: buttonIcon

        anchors.fill: parent
        iconCode: IconCode.MENU_THREE_DOTS
    }

    MouseArea {
        id: menuButtonMouseArea

        anchors.fill: parent

        onClicked: {
            if (!opened) {
                contextMenu.open()
            } else {
                contextMenu.close()
            }
        }

        hoverEnabled: true

        onEntered: {
            root.hovered = true
        }

        onExited: {
            root.hovered = false
        }
    }

    Menu {
        id: contextMenu

        x: root.width - width
        y: root.y + root.height

        height: 120
        width: 200

        closePolicy: Popup.CloseOnEscape | Popup.CloseOnReleaseOutsideParent

        background: DropShadow {
            anchors.fill: parent
            verticalOffset: 6
            radius: 12.0
            samples: 30
            color: "#75000000"
            source: contextMenu.contentItem
        }

        bottomPadding: 4
        topPadding: 4

        onOpened: {
            root.opened = true
        }

        onClosed: {
            root.opened = false
        }
    }

    Component {
        id: menuItemComponent

        StyledMenuItem {
            id: menuItem

            implicitHeight: 30
            implicitWidth: contextMenu.width
        }
    }

    states: [

        State {
            name: "HOVERED"
            when: root.hovered && !root.opened
            PropertyChanges {
                target: root
                color: ui.theme.buttonColor
                opacity: ui.theme.buttonOpacityHover
            }
        },
        State {
            name: "PRESSED"
            when: root.opened
            PropertyChanges {
                target: root
                color: ui.theme.accentColor
                opacity: ui.theme.accentOpacityHit
            }
        }
    ]
}
