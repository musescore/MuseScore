import QtQuick 2.0

import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.Shortcuts 1.0

Item {
    id: root

    property string actionCode

    signal itemHandled()

    ToolbarShortcutsContextModel {
        id: model
    }

    ContextMenuLoader {
        id: menu

        items: [
            { id: "add", title: qsTrc("notation", "Assign Shortcut"), icon: IconCode.CONFIGURE, enabled: true },
            { id: "remove", title: qsTrc("notation","Clear current shortcut"), icon: IconCode.DELETE_TANK, enabled: true }
        ]
        onHandleMenuItem: function(itemId) {
            console.log("Hit " + actionCode + " with " + itemId)

            switch(itemId) {
            case "add":
                model.addShortcut(actionCode)
                break
            case "remove":
                model.removeShortcut(actionCode)
                break
            }

            root.itemHandled()
        }
    }

    function show(mouseLoc : point) {
        menu.show(mouseLoc)
    }
}
