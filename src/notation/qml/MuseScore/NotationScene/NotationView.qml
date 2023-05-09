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

    property alias name: notationView.objectName
    property alias publishMode: notationView.publishMode

    property alias paintView: notationView

    property alias isNavigatorVisible: notationNavigator.visible
    property alias isMainView: notationView.isMainView

    property alias defaultNavigationControl: fakeNavCtrl

    property NavigationPanel navigationPanel: notationView.navigationPanel // first panel

    NavigationSection {
        id: navSec
        name: "NotationView"
        order: 5
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
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        NotationSwitchPanel {
            id: tabPanel
            Layout.fillWidth: true

            navigationPanel.section: navSec
            navigationPanel.order: notationView.navigationPanel.order + 1
        }

        SeparatorLine { visible: tabPanel.visible }

        SplitView {
            id: splitView

            Layout.fillWidth: true
            Layout.fillHeight: true

            orientation: notationNavigator.orientation === Qt.Horizontal ? Qt.Vertical : Qt.Horizontal

            NotationScrollAndZoomArea {
                SplitView.fillWidth: true
                SplitView.fillHeight: true

                NotationPaintView {
                    id: notationView
                    anchors.fill: parent

                    property NavigationPanel navigationPanel: NavigationPanel {
                        name: "ScoreView"
                        section: navSec
                        enabled: notationView.enabled && notationView.visible
                        direction: NavigationPanel.Both
                        order: 1
                    }

                    NavigationControl {
                        id: fakeNavCtrl
                        name: "Score"
                        enabled: notationView.enabled && notationView.visible

                        panel: navigationPanel
                        order: 1

                        onActiveChanged: {
                            if (fakeNavCtrl.active) {
                                notationView.forceFocusIn()

                                if (navigationPanel.highlight) {
                                    notationView.selectOnNavigationActive()
                                }
                            } else {
                                notationView.focus = false
                            }
                        }
                    }

                    NavigationFocusBorder {
                        navigationCtrl: fakeNavCtrl
                        drawOutsideParent: false
                    }

                    onActiveFocusRequested: {
                        fakeNavCtrl.requestActive()
                    }

                    onShowContextMenuRequested: function (elementType, viewPos) {
                        contextMenuModel.loadItems(elementType)
                        contextMenuLoader.show(viewPos, contextMenuModel.items)
                    }

                    onHideContextMenuRequested: {
                        contextMenuLoader.close()
                    }

                    onShowElementPopupRequested: function (popupType, viewPos, elemSize) {
                        popUpLoader.show(popupType, viewPos, elemSize)
                    }

                    onHideElementPopupRequested: {
                        popUpLoader.close()
                    }

                    onViewportChanged: {
                        notationNavigator.setCursorRect(viewport)
                    }

                    ContextMenuLoader {
                        id: contextMenuLoader

                        onHandleMenuItem: function(itemId) {
                            contextMenuModel.handleMenuItem(itemId)
                        }
                    }

                    ElementPopupLoader {
                        id: popUpLoader

                        navigationSection: navSec
                        navigationOrderStart: 2

                        onOpened: paintView.setIsPopupOpen(true)
                        onClosed: paintView.setIsPopupOpen(false)
                    }
                }
            }

            Loader {
                id: notationNavigator

                property var orientation: notationNavigator.item ? notationNavigator.item.orientation : Qt.Horizontal

                visible: false

                SplitView.preferredHeight: 100
                SplitView.preferredWidth: 100
                SplitView.minimumWidth: 30
                SplitView.minimumHeight: 30

                sourceComponent: notationNavigator.visible ? navigatorComp : null

                function setCursorRect(viewport) {
                    if (notationNavigator.item) {
                        notationNavigator.item.setCursorRect(viewport)
                    }
                }
            }

            Component {
                id: navigatorComp

                NotationNavigator {

                    property bool isVertical: orientation === Qt.Vertical

                    objectName: root.name + "Navigator"

                    Component.onCompleted: {
                        load()
                    }

                    onMoveNotationRequested: function(dx, dy) {
                        notationView.moveCanvas(dx, dy)
                    }
                }
            }

            handle: Rectangle {
                id: resizingHandle

                implicitWidth: 4
                implicitHeight: 4

                color: ui.theme.strokeColor

                states: [
                    State {
                        name: "PRESSED"
                        when: resizingHandle.SplitHandle.pressed
                        PropertyChanges {
                            target: resizingHandle
                            opacity: ui.theme.accentOpacityHit
                        }
                    },
                    State {
                        name: "HOVERED"
                        when: resizingHandle.SplitHandle.hovered
                        PropertyChanges {
                            target: resizingHandle
                            opacity: ui.theme.accentOpacityHover
                        }
                    }
                ]
            }
        }

        SearchPopup {
            id: searchPopup

            Layout.fillWidth: true

            navigationPanel.section: navSec
            navigationPanel.order: tabPanel.navigationPanel.order + 1

            onClosed: {
                fakeNavCtrl.requestActive()
            }
        }
    }
}
