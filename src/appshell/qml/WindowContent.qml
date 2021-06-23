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
import QtQuick.Controls 2.15

import MuseScore.AppShell 1.0
import MuseScore.Dock 1.0
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

import "./HomePage"
import "./NotationPage"
import "./PublishPage"
import "./DevTools"
import "./dockwindow"

DockWindow {
    id: root

    objectName: "WindowContent"

    property var provider: InteractiveProvider {
        topParent: root

        onRequestedDockPage: {
            root.loadPage(uri)
        }
    }

    property NavigationSection topToolKeyNavSec: NavigationSection {
        id: keynavSec
        name: "TopTool"
        order: 1
    }

    toolBars: [
        DockToolBar {
            id: mainToolBar

            objectName: "mainToolBar"
            title: qsTrc("appshell", "Main Toolbar")

            width: root.width / 2
            minimumWidth: 304

            movable: false

            contentComponent: MainToolBar {
                navigation.section: root.topToolKeyNavSec
                navigation.order: 1

                currentUri: root.currentPageUri

                navigation.onActiveChanged: {
                    if (navigation.active) {
                        mainToolBar.forceActiveFocus()
                    }
                }

                onSelected: {
                    api.launcher.open(uri)
                }
            }
        }
    ]

    mainToolBarDockingHolder: DockToolBarHolder {
        objectName: root.objectName + "_mainToolBarDockingHolderTop"
        location: DockBase.Top

        Rectangle { color: ui.theme.backgroundPrimaryColor }
    }

    pages: [
        HomePage {},

        NotationPage {
            dockWindow: root

            topToolKeyNavSec: root.topToolKeyNavSec
        },

        PublishPage {
            topToolKeyNavSec: root.topToolKeyNavSec
        },

        DevToolsPage {}
    ]
}
