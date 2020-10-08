import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15

import MuseScore.NotationScene 1.0

FocusScope {
    ColumnLayout {
        anchors.fill: parent

        spacing: 0

        NotationSwitchPanel {
            Layout.fillWidth: true
        }

        NotationPaintView {
            id: notationView

            Layout.fillWidth: true
            Layout.fillHeight: true

            onOpenContextMenuRequested: {
                contextMenu.clear()

                for (var i in items) {
                    var item = items[i]

                    // TODO: move to a stylized menu
                    var menuItemObj = menuItem.createObject(notationView, { name: item.name, text: item.title })
                    menuItemObj._triggered.connect(function(name) {
                        notationView.handleAction(name)
                    })
                    contextMenu.addItem(menuItemObj)
                }

                contextMenu.popup(pos.x, pos.y)
            }
        }

        Menu {
            id: contextMenu

            // TODO: remove after replacing with a stylized menu
            function clear() {
                for (var i = contextMenu.contentData.length - 1; i >= 0; --i) {
                    removeItem(i)
                }
            }

            Component {
                id: menuItem
                MenuItem {
                    property string name: ""
                    signal _triggered(string name)

                    onTriggered: {
                        _triggered(name)
                    }
                }
            }
        }
    }
}
