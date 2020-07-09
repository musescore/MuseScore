import QtQuick 2.7
import MuseScore.Dock 1.0
import MuseScore.Ui 1.0

import "./HomePage"
import "./NotationPage"
import "./Settings"
import "./DevTools"

DockWindow {

    id: dockWindow

    title: qsTrc("appshell", "MuseScore 4")

    color: ui.theme.backgroundColor

    currentPageUri: "musescore://notation"

    property var provider: LaunchProvider {
        topParent: dockWindow
        resolver: LaunchResolver{}
        onRequestedDockPage: {
            dockWindow.currentPageUri = uri
        }
    }

    toolbars: [
        DockToolBar {
            id: mainToolBar
            objectName: "mainToolBar"

            width: dockWindow.width/2
            height: 32
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
            id: playToolBar
            objectName: "playToolBar"

            width: dockWindow.width / 2
            height: 32
            color: dockWindow.color

            PlayToolBar {
                color: dockWindow.color
            }
        }
    ]

    HomePage {
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


