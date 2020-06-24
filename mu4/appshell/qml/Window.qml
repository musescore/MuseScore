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

    currentPageName: "home"

    property var provider: LaunchProvider {
        onRequestedPage: {
            console.log("onRequestedPage: " + uri)
            dockWindow.currentPageName = uri
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
                currentItem: dockWindow.currentPageName
                onSelected: {
                   // dockWindow.currentPageName = item;
                    api.launcher.open(item)
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

    }

    NotationPage {

    }

    SequencerPage {

    }

    PublishPage {

    }

    SettingsPage {

    }
}
