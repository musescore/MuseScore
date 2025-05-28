/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.Dock 1.0

import MuseScore.AppShell 1.0

import "../"
import "../../"
import "../../NotationPage"

AppWindow {
    id: root

    flags: Qt.FramelessWindowHint

    AppMenuBar {
        id: appMenuBar
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
    }

    Component.onCompleted: {
        dockwin.init()
    }

    // WindowContent {
    //     id: dockwin

    //     anchors.top: appMenuBar.bottom
    //     anchors.left: parent.left
    //     anchors.right: parent.right
    //     anchors.bottom: parent.bottom
    // }

    DockWindow {
        id: dockwin

        anchors.top: appMenuBar.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        objectName: "WindowContent"

        onPageLoaded: {
            interactiveProvider.onPageOpened()
        }

        InteractiveProvider {
            id: interactiveProvider
            topParent: dockwin

            onRequestedDockPage: function(uri, params) {
                dockwin.loadPage(uri, params)
            }
        }

        NavigationSection {
            id: topToolbarKeyNavSec
            name: "TopTool"
            order: 1
        }

        // toolBars: [
        //     DockToolBar {
        //         id: mainToolBar

        //         objectName: "mainToolBar"
        //         title: qsTrc("appshell", "Main toolbar")

        //         floatable: false
        //         closable: false

        //         navigationSection: topToolbarKeyNavSec

        //         MainToolBar {
        //             id: toolBar
        //             navigation.section: mainToolBar.navigationSection
        //             navigation.order: 1

        //             currentUri: dockwin.currentPageUri

        //             navigation.onActiveChanged: {
        //                 if (navigation.active) {
        //                     mainToolBar.forceActiveFocus()
        //                 }
        //             }

        //             onSelected: function(uri) {
        //                 api.launcher.open(uri)
        //             }

        //             Component.onCompleted: {
        //                 toolBar.focusOnFirst()
        //             }
        //         }
        //     }
        // ]

        pages: [
            // HomePage {
            //     window: root.window
            // },

            NotationPage {
                topToolbarKeyNavSec: topToolbarKeyNavSec
            }//,

            //         PublishPage {
            //             topToolbarKeyNavSec: topToolbarKeyNavSec
            //         },

            //         DevToolsPage {}
        ]
    }
}
