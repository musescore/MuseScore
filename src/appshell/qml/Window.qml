import QtQuick 2.7

import MuseScore.Dock 1.0
import MuseScore.Ui 1.0
import MuseScore.Playback 1.0
import MuseScore.NotationScene 1.0
import MuseScore.AppMenu 1.0
import MuseScore.Shortcuts 1.0

import "./HomePage"
import "./NotationPage"
import "./Settings"
import "./DevTools"

DockWindow {
    id: dockWindow

    title: qsTrc("appshell", "MuseScore 4")

    color: ui.theme.backgroundPrimaryColor
    borderColor: ui.theme.strokeColor

    Component.onCompleted: {
        shortcutsModel.load()
        appMenuModel.load()
        api.launcher.open(homePage.uri)
    }

    property var provider: InteractiveProvider {
        topParent: dockWindow
        onRequestedDockPage: {
            dockWindow.currentPageUri = uri
        }
    }

    readonly property int toolbarHeight: 48
    property bool isNotationPage: currentPageUri === notationPage.uri

    property ShortcutsInstanceModel shortcutsModel: ShortcutsInstanceModel {}
    property AppMenuModel appMenuModel: AppMenuModel {}

    menuBar: DockMenuBar {
        objectName: "mainMenuBar"

        items: appMenuModel.items

        onActionTringgered: {
            appMenuModel.handleAction(action)
        }
    }

    toolbars: [
        DockToolBar {
            objectName: "mainToolBar"
            minimumWidth: 376
            minimumHeight: dockWindow.toolbarHeight

            color: dockWindow.color
            allowedAreas: Qt.TopToolBarArea

            content: MainToolBar {
                color: dockWindow.color
                currentUri: dockWindow.currentPageUri

                onSelected: {
                    api.launcher.open(uri)
                }
            }
        },

        DockToolBar {
            objectName: "notationToolBar"
            minimumWidth: 188
            minimumHeight: dockWindow.toolbarHeight

            color: dockWindow.color
            allowedAreas: Qt.TopToolBarArea
            visible: dockWindow.isNotationPage

            content: NotationToolBar {
                color: dockWindow.color
            }
        },

        DockToolBar {
            id: playbackToolBar

            objectName: "playbackToolBar"
            minimumWidth: floating ? 492 : 420
            minimumHeight: floating ? 76 : dockWindow.toolbarHeight

            color: dockWindow.color
            allowedAreas: Qt.TopToolBarArea
            visible: dockWindow.isNotationPage

            content: PlaybackToolBar {
                color: dockWindow.color
                floating: playbackToolBar.floating
            }
        },

        DockToolBar	{
            objectName: "undoRedoToolBar"

            minimumWidth: 72
            minimumHeight: dockWindow.toolbarHeight

            color: dockWindow.color
            visible: dockWindow.isNotationPage
            floatable: false
            movable: false

            content: UndoRedoToolBar {
                color: dockWindow.color
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
