import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0

import "internal"

FocusScope {
    id: root

    signal textEdittingStarted()

    QtObject {
        id: privateProperties

        property int scrollbarMargin: 4
    }

    ColumnLayout {
        anchors.fill: parent

        spacing: 0

        NotationSwitchPanel {
            Layout.fillWidth: true
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            NotationPaintView {
                id: notationView

                anchors.fill: parent

                onTextEdittingStarted: {
                    root.textEdittingStarted()
                }

                Component {
                    id: menuAction

                    Action {
                        property string code: ""
                        property string hintIcon: ""

                        icon.name: hintIcon

                        onTriggered: {
                            Qt.callLater(notationView.handleAction, code)
                        }
                    }
                }

                onOpenContextMenuRequested: {
                    contextMenu.clear()

                    for (var i in items) {
                        var item = items[i]

                        var action = menuAction.createObject(notationView, {
                                                             code: item.code,
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

                onHorizontalScrollChanged: {
                    if (!horizontalScrollBar.pressed) {
                        horizontalScrollBar.setPosition(notationView.startHorizontalScrollPosition)
                    }
                }

                onVerticalScrollChanged: {
                    if (!verticalScrollBar.pressed) {
                        verticalScrollBar.setPosition(notationView.startVerticalScrollPosition)
                    }
                }
            }

            StyledScrollBar {
                id: verticalScrollBar

                anchors.top: parent.top
                anchors.bottomMargin: privateProperties.scrollbarMargin
                anchors.bottom: parent.bottom
                anchors.right: parent.right

                orientation: Qt.Vertical

                color: "black"
                withBorder: true
                borderColor: "white"

                size: notationView.verticalScrollSize

                onPositionChanged: {
                    if (pressed) {
                        notationView.scrollVertical(position)
                    }
                }
            }

            StyledScrollBar {
                id: horizontalScrollBar

                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
                anchors.rightMargin: privateProperties.scrollbarMargin

                orientation: Qt.Horizontal

                color: "black"
                withBorder: true
                borderColor: "white"

                size: notationView.horizontalScrollSize

                onPositionChanged: {
                    if (pressed) {
                        notationView.scrollHorizontal(position)
                    }
                }
            }
        }

        SearchPopup {
            id: searchPopup

            Layout.fillWidth: true
        }
    }

    Component.onCompleted: {
        notationView.load()
    }
}
