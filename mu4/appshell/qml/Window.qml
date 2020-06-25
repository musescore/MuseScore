import QtQuick 2.7
import MuseScore.Dock 1.0
import MuseScore.Ui 1.0

import "./HomePage"
import "./NotationPage"
import "./Settings"

DockWindow {

    id: dockWindow

    title: qsTrc("appshell", "MuseScore 4")

    color: ui.theme.window

    currentPageUri: "musescore://home"

    property var provider: LaunchProvider {
        onRequestedPage: {
            console.log("onRequestedPage: " + uri)
            dockWindow.currentPageUri = uri
        }
    }

    toolbars: [
        DockToolBar {
            id: mainToolBar
            objectName: "mainToolBar"

            width: 300
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

            width: 300
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
}
