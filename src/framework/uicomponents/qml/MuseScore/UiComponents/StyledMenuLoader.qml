import QtQuick 2.15

Loader {

    id: loader

    signal handleAction(string actionCode, int actionIndex)

    property alias menu: loader.item

    function isMenuOpened() {
        return loader.menu && loader.menu.isOpened
    }

    function toggleOpened(model) {
        if (!loader.sourceComponent) {
            loader.sourceComponent = itemMenuComp
        }

        var menu = loader.menu
        if (menu.isOpened) {
            menu.close()
            return
        }

        menu.parent = loader.parent
        menu.model = model
        menu.open()
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
