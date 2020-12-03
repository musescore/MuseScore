import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15

import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0

import "internal"

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

            Component {
                id: menuAction

                Action {
                    property string name: ""
                    property string hintIcon: ""

                    icon.name: hintIcon

                    onTriggered: {
                        Qt.callLater(notationView.handleAction, name)
                    }
                }
            }

            onOpenContextMenuRequested: {
                contextMenu.clear()

                for (var i in items) {
                    var item = items[i]

                    var action = menuAction.createObject(notationView, {
                                                         name: item.name,
                                                         text: item.title,
                                                         hintIcon: item.icon,
                                                         shortcut: item.shortcut
                                                         })
                    contextMenu.addMenuItem(action)
                }

                contextMenu.popup()
            }

            ContextMenu {
                id: contextMenu
            }
        }

        SearchPopup {
            id: searchPopup

            Layout.fillWidth: true
        }
    }
}
