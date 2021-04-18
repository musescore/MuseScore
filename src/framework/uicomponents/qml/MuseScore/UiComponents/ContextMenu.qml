import QtQuick 2.9
import QtQuick.Controls 2.15

Menu {
    id: root

    closePolicy: Popup.CloseOnEscape | Popup.CloseOnReleaseOutsideParent | Popup.CloseOnPressOutside

    implicitWidth: 220

    bottomPadding: 4
    topPadding: bottomPadding

    function clear() {
        for (var i = root.contentData.length - 1; i >= 0; --i) {
            root.removeItem(i)
        }
    }

    function addMenuItem(itemAction) {
        var obj = menuItem.createObject(root.parent, { action: itemAction })
        addItem(obj)
    }

    background: Rectangle {
        radius: 3

        border.width: 1
        border.color: ui.theme.strokeColor

        color: ui.theme.popupBackgroundColor
    }

    Component {
        id: menuItem

        StyledContextMenuItem {
            hintIcon: Boolean(action) ? action.icon.name : ""
        }
    }
}
