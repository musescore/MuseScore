/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

import QtQuick

import Muse.Ui
import Muse.UiComponents
import Muse.Interactive
import Muse.Dock

import MuseScore.AppShell

AppWindow {
    id: root

    flags: Qt.FramelessWindowHint

    AppMenuBar {
        id: appMenuBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
    }

    DockWindow {
        id: dockWindow

        anchors.top: appMenuBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        objectName: "WindowContent"

        Component.onCompleted: {
            dockWindow.init()
        }

        onPageLoaded: {
            interactiveProvider.onPageOpened()
        }

        InteractiveProvider {
            id: interactiveProvider
            topParent: dockWindow

            onRequestedDockPage: function(uri, params) {
                dockWindow.loadPage(uri, params)
            }
        }

        NavigationSection {
            id: topToolbarKeyNavSec
            name: "TopTool"
            order: 1
        }

        toolBars: [
            DockToolBar {
                id: mainToolBar

                objectName: "mainToolBar"
                title: qsTrc("appshell", "Main toolbar")

                floatable: false
                closable: false

                navigationSection: topToolbarKeyNavSec

                PageTabBar {
                    id: toolBar

                    model: MainToolBarModel {}

                    navigation.name: "MainToolBar"
                    accessibleName: qsTrc("appshell", "Main toolbar")

                    navigation.section: mainToolBar.navigationSection
                    navigation.order: 1

                    currentUri: dockWindow.currentPageUri

                    navigation.onActiveChanged: {
                        if (navigation.active) {
                            mainToolBar.forceActiveFocus()
                        }
                    }

                    onSelected: function(uri) {
                        dockWindow.openPage(uri)
                    }

                    Component.onCompleted: {
                        toolBar.focusOnFirst()
                    }
                }
            }
        ]

        pages: [
            NotationPage {
                topToolbarKeyNavSec: topToolbarKeyNavSec
            },

            DockPage {
                objectName: "DevTools"
                uri: "musescore://devtools"

                central: DevFrame {}
            }
        ]
    }
}
