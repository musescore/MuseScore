import QtQuick 2.15

Loader {

    id: loader

    signal handleAction(string actionCode, int actionIndex)

    property alias menu: loader.item

    function isMenuOpened() {
        return loader.menu && loader.menu.isOpened
    }

    function toggleOpened(model, keynavParentControl) {
        if (!loader.sourceComponent) {
            loader.sourceComponent = itemMenuComp
        }

        var menu = loader.menu
        if (menu.isOpened) {
            menu.close()
            return
        }

        menu.parent = loader.parent
        if (keynavParentControl) {
            menu.keynav.parentControl = keynavParentControl
            menu.keynav.name = keynavParentControl.name+"PopupMenu"
        }
        menu.model = model
        menu.open()

        if (!menu.focusOnSelected()) {
            menu.focusOnFirstItem()
        }
    }

    Component {
        id: itemMenuComp
        StyledMenu {
            id: itemMenu
            onHandleAction: {
                Qt.callLater(loader.handleAction, actionCode, actionIndex)
                itemMenu.close()
            }
        }
    }
}
