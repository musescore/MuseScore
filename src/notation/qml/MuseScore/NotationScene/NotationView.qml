/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.NotationScene 1.0

import "internal"

FocusScope {
    id: root

    property alias isNavigatorVisible: notationNavigator.visible

    signal textEdittingStarted()

    NavigationSection {
        id: navSec
        name: "NotationView"
        order: 4
        enabled: root.visible
    }

    QtObject {
        id: prv
        readonly property int scrollbarMargin: 4
    }

    NotationContextMenuModel {
        id: contextMenuModel
    }

    Component.onCompleted: {
        notationView.load()
        notationNavigator.load()
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        NotationSwitchPanel {
            id: tabPanel
            Layout.fillWidth: true

            navigationSection: navSec
        }

        SplitView {
            id: splitView

            Layout.fillWidth: true
            Layout.fillHeight: true

            orientation: notationNavigator.orientation === Qt.Horizontal ? Qt.Vertical : Qt.Horizontal

            background: Rectangle {
                color: notationView.backgroundColor
            }

            NotationPaintView {
                id: notationView

                NavigationPanel {
                    id: navPanel
                    name: "ScoreView"
                    section: navSec
                    direction: NavigationPanel.Both
                    order: 2
                }

                NavigationControl {
                    id: fakeNavCtrl
                    name: "Score"

                    panel: navPanel
                    order: 1

                    accessible.role: MUAccessible.Panel
                    accessible.name: "Score"

                    onActiveChanged: {
                        if (fakeNavCtrl.active) {
                            notationView.selectOnNavigationActive()
                        }
                    }
                }

                SplitView.fillWidth: true
                SplitView.fillHeight: true

                onTextEdittingStarted: {
                    root.textEdittingStarted()
                }

                onShowContextMenuRequested: function (elementType, pos) {
                    contextMenuModel.loadItems(elementType)

                    if (contextMenuLoader.isMenuOpened) {
                        contextMenuLoader.update(contextMenuModel.items, pos.x, pos.y)
                    } else {
                        // TODO: replace `null` with a NavigationControl
                        contextMenuLoader.open(contextMenuModel.items, null, pos.x, pos.y)
                    }
                }

                onHideContextMenuRequested: function() {
                    contextMenuLoader.close()
                }

                onViewportChanged: {
                    notationNavigator.setCursorRect(viewport)
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

                StyledScrollBar {
                    id: verticalScrollBar

                    anchors.top: parent.top
                    anchors.bottomMargin: prv.scrollbarMargin
                    anchors.bottom: parent.bottom
                    anchors.right: parent.right

                    orientation: Qt.Vertical

                    color: "black"
                    border.width: 1
                    border.color: "white"

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
                    anchors.rightMargin: prv.scrollbarMargin

                    orientation: Qt.Horizontal

                    color: "black"
                    border.width: 1
                    border.color: "white"

                    size: notationView.horizontalScrollSize

                    onPositionChanged: {
                        if (pressed) {
                            notationView.scrollHorizontal(position)
                        }
                    }
                }

                StyledMenuLoader {
                    id: contextMenuLoader

                    onHandleMenuItem: function (itemId) {
                        contextMenuModel.handleMenuItem(itemId)
                    }
                }
            }

            NotationNavigator {
                id: notationNavigator

                property bool isVertical: orientation === Qt.Vertical

                visible: false

                SplitView.preferredHeight: 100
                SplitView.preferredWidth: 100

                onMoveNotationRequested: function (dx, dy) {
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
}
