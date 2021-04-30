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
import QtQuick 2.7

import MuseScore.Dock 1.0
import MuseScore.Ui 1.0
import MuseScore.Playback 1.0
import MuseScore.NotationScene 1.0
import MuseScore.AppShell 1.0
import MuseScore.Shortcuts 1.0

import "./HomePage"
import "./NotationPage"
import "./DevTools"

DockWindow {
    id: dockWindow

    title: qsTrc("appshell", "MuseScore 4")

    color: ui.theme.backgroundPrimaryColor
    borderColor: ui.theme.strokeColor

    Component.onCompleted: {
        shortcutsModel.load()
        appMenuModel.load()
        startupModel.load()
    }

    property var shortcuts: Shortcuts{}

    property var provider: InteractiveProvider {
        topParent: dockWindow
        onRequestedDockPage: {
            initParams(uri, params)
            dockWindow.currentPageUri = uri
        }

        function initParams(uri, params) {
            if (!Boolean(params)) {
                return
            }

            for (var i in dockWindow.pages) {
                var page = dockWindow.pages[i]
                if (page.uri !== uri) {
                    continue
                }

                var props = Object.keys(page)
                for (var p in params) {
                    if (props.indexOf(p) !== -1) {
                        var value = params[p]
                        // NOTE: needed reset the value for emitting a signal with new value
                        page[p] = null
                        page[p] = value
                    }
                }
            }
        }
    }

    readonly property int toolbarHeight: 48
    property bool isNotationPage: currentPageUri === notationPage.uri

    property ShortcutsInstanceModel shortcutsModel: ShortcutsInstanceModel {}
    property AppMenuModel appMenuModel: AppMenuModel {}
    property StartupModel startupModel: StartupModel {}

    property NavigationSection topToolNavSec: NavigationSection {
        name: "TopTool"
        order: 1
    }

    menuBar: DockMenuBar {
        objectName: "mainMenuBar"

        items: appMenuModel.items

        onActionTriggered: {
            appMenuModel.handleAction(actionCode, actionIndex)
        }
    }

    toolbars: [
        DockToolBar {
            id: mainToolBar
            objectName: "mainToolBar"
            minimumWidth: 296
            minimumHeight: dockWindow.toolbarHeight

            color: dockWindow.color
            allowedAreas: Qt.TopToolBarArea

            title: qsTrc("appshell", "Main Toolbar")

            content: MainToolBar {
                color: dockWindow.color
                navigation.section: topToolNavSec
                navigation.order: 1
                currentUri: dockWindow.currentPageUri

                navigation.onActiveChanged: {
                    if (navigation.active) {
                        mainToolBar.forceActiveFocus()
                    }
                }

                onSelected: {
                    api.launcher.open(uri)
                }
            }
        },

        DockToolBar {
            id: notationToolBar
            objectName: "notationToolBar"
            minimumWidth: 192
            minimumHeight: dockWindow.toolbarHeight

            color: dockWindow.color
            allowedAreas: Qt.TopToolBarArea

            title: qsTrc("appshell", "Notation Toolbar")

            content: NotationToolBar {
                id: notationToolBarContent
                color: dockWindow.color

                navigation.section: topToolNavSec
                navigation.order: 2
                navigation.enabled: notationToolBar.visible
                onActiveFocusRequested: {
                    if (navigation.active) {
                        notationToolBar.forceActiveFocus()
                    }
                }

                Connections {
                    target: notationToolBar

                    Component.onCompleted: {
                        notationPage.pageModel.isNotationToolBarVisible = notationToolBar.visible
                        notationToolBar.visible = Qt.binding(function() { return dockWindow.isNotationPage && notationPage.pageModel.isNotationToolBarVisible })
                    }
                }
            }
        },

        DockToolBar {
            id: playbackToolBar

            objectName: "playbackToolBar"
            minimumWidth: floating ? 520 : 470
            minimumHeight: floating ? 76 : dockWindow.toolbarHeight

            color: dockWindow.color
            allowedAreas: Qt.TopToolBarArea

            title: qsTrc("appshell", "Playback Controls")

            content: PlaybackToolBar {
                id: playbackToolBarContent
                color: dockWindow.color

                navigation.section: topToolNavSec
                navigation.order: 3
                navigation.enabled: dockWindow.isNotationPage

                floating: playbackToolBar.floating

                Connections {
                    target: playbackToolBar

                    Component.onCompleted: {
                        notationPage.pageModel.isPlaybackToolBarVisible = playbackToolBar.visible
                        playbackToolBar.visible = Qt.binding(function() { return dockWindow.isNotationPage && notationPage.pageModel.isPlaybackToolBarVisible})
                    }
                }
            }
        },

        DockToolBar {
            id: undoRedoToolBar
            objectName: "undoRedoToolBar"

            minimumWidth: 72
            minimumHeight: dockWindow.toolbarHeight

            color: dockWindow.color
            floatable: false
            movable: false

            title: qsTrc("appshell", "Undo/Redo Toolbar")

            content: UndoRedoToolBar {
                id: undoRedoToolBarContent
                color: dockWindow.color

                Connections {
                    target: undoRedoToolBar

                    Component.onCompleted: {
                        notationPage.pageModel.isUndoRedoToolBarVisible = undoRedoToolBar.visible
                        undoRedoToolBar.visible = Qt.binding(function() { return dockWindow.isNotationPage && notationPage.pageModel.isUndoRedoToolBarVisible})
                    }
                }
            }
        }
    ]

    HomePage {
        id: homePage

        uri: "musescore://home"
    }

    NotationPage {
        id: notationPage

        uri: "musescore://notation"

        isNotationToolBarVisible: Boolean(currentPageUri) && notationToolBar.visible
        isPlaybackToolBarVisible: Boolean(currentPageUri) && playbackToolBar.visible
        isUndoRedoToolBarVisible: Boolean(currentPageUri) && undoRedoToolBar.visible

        Connections {
            target: dockWindow

            function onCurrentPageUriChanged(currentPageUri) {
                notationPage.updatePageState()
            }
        }
    }

    SequencerPage {
        uri: "musescore://sequencer"
    }

    PublishPage {
        uri: "musescore://publish"
    }

    DevToolsPage {
        uri: "musescore://devtools"
    }
}
