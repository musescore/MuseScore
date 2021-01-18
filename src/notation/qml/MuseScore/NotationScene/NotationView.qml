import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import MuseScore.NotationScene 1.0
import MuseScore.UiComponents 1.0

import "internal"

FocusScope {
    id: root

    signal textEdittingStarted()

    ColumnLayout {
        anchors.fill: parent

        spacing: 0

        NotationSwitchPanel {
            Layout.fillWidth: true
        }

        SplitView {
            id: splitView

            Layout.fillWidth: true
            Layout.fillHeight: true

            orientation: notationNavigator.orientation

            NotationPaintView {
                id: notationView

                SplitView.fillWidth: true
                SplitView.fillHeight: true

                onTextEdittingStarted: {
                    root.textEdittingStarted()
                }

                onOpenContextMenuRequested: {
                    privateProperties.showNotationMenu(items)
                }

                onViewportChanged: {
                    notationNavigator.setViewRect(viewport)
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

                ContextMenu {
                    id: contextMenu
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

            NotationNavigator {
                id: notationNavigator

                property bool isVertical: orientation === Qt.Vertical

                SplitView.preferredHeight: 200
                SplitView.preferredWidth: 200

                onMoveNotationRequested: {
                    notationView.moveCanvas(dx, dy)
                }
            }

            handle: Rectangle {
                id: background

                implicitWidth: 4
                implicitHeight: 4

                color: ui.theme.strokeColor

                states: [
                    State {
                        name: "PRESSED"
                        when: background.SplitHandle.pressed
                        PropertyChanges {
                            target: background
                            opacity: ui.theme.accentOpacityHit
                        }
                    },
                    State {
                        name: "HOVERED"
                        when: background.SplitHandle.hovered
                        PropertyChanges {
                            target: background
                            opacity: ui.theme.accentOpacityHover
                        }
                    }
                ]
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

    QtObject {
        id: privateProperties

        property int scrollbarMargin: 4

        function showNotationMenu(items) {
            contextMenu.clear()

            for (var i in items) {
                var item = items[i]

                var action = notationMenuAction.createObject(notationView, {
                                                                 code: item.code,
                                                                 text: item.title,
                                                                 hintIcon: item.icon,
                                                                 shortcut: item.shortcut
                                                             })
                contextMenu.addMenuItem(action)
            }

            contextMenu.popup()
        }
    }

    Component {
        id: notationMenuAction

        Action {
            property string code: ""
            property string hintIcon: ""

            icon.name: hintIcon

            onTriggered: {
                Qt.callLater(notationView.handleAction, code)
            }
        }
    }
}
