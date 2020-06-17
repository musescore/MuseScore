import QtQuick 2.7
import MuseScore.Dock 1.0

import "./HomePage"
import "./NotationPage"
import "./Settings"

DockWindow {

    id: dockWindow

    title: "MuseScore Craft"

    color: ui.theme.window

    currentPageName: "home"

    toolbar: DockToolBar {

        id: windowToolBar
        objectName: "windowToolBar"

        width: 300
        height: 32
        color: dockWindow.color

        MainToolBar {
            color: windowToolBar.color
            currentItem: dockWindow.currentPageName
            onSelected: {
                dockWindow.currentPageName = item;
            }
        }
    }


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
