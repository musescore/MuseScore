import QtQuick 2.7
import MuseScore.Dock 1.0
import MuseScore.Ui 1.0
import MuseScore.Playback 1.0
import MuseScore.NotationScene 1.0

import "./HomePage"
import "./NotationPage"
import "./Settings"
import "./DevTools"

DockWindow {

    id: dockWindow

    title: qsTrc("appshell", "MuseScore 4")

    color: ui.theme.backgroundPrimaryColor

    Component.onCompleted: {
        api.launcher.open(home.uri)
    }

    property var provider: InteractiveProvider {
        topParent: dockWindow
        onRequestedDockPage: {
            dockWindow.currentPageUri = uri
        }
    }

    toolbars: [
        DockToolBar {
            id: mainToolBar
            objectName: "mainToolBar"

            minimumWidth: 600
            minimumHeight: 48
            color: dockWindow.color

            MainToolBar {
                color: dockWindow.color
                currentUri: dockWindow.currentPageUri
                onSelected: {
                    api.launcher.open(uri)
                }
            }
        },

        DockToolBar {
            id: notationToolBar
            objectName: "notationModeToolBar"

            minimumWidth: 200
            minimumHeight: 48
            color: dockWindow.color

            visible: dockWindow.currentPageUri !== home.uri

            NotationModeToolBar {
                color: dockWindow.color
            }
        },

        DockToolBar {
            id: playToolBar
            objectName: "playToolBar"

            minimumWidth: 100
            minimumHeight: 48
            color: dockWindow.color

            visible: dockWindow.currentPageUri !== home.uri

            PlaybackToolBar {
                color: dockWindow.color
            }
        }
    ]

    HomePage {
        id: home

        uri: "musescore://home"
    }

    NotationPage {
        uri: "musescore://notation"
    }

    SequencerPage {
        uri: "musescore://sequencer"
    }

    PublishPage {
        uri: "musescore://publish"
    }

    SettingsPage {
        uri: "musescore://settings"
    }

    DevToolsPage {
        uri: "musescore://devtools"
    }
}
