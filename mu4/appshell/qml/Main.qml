import QtQuick 2.9
import MuseScore.Dock 1.0

import "./HomePage"
import "./NotationPage"

DockWindow {

    id: dockWindow

    title: "MuseScore Craft"

    color: ui.theme.window

    currentPageName: "notation"

    toolbar: DockToolBar {

        id: windowToolBar
        objectName: "windowToolBar"

        width: 300
        height: 28
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

}
