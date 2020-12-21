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

            minimumWidth: 900
            minimumHeight: 48

            color: dockWindow.color
            allowedAreas: Qt.TopToolBarArea

            content: Rectangle {
                color: dockWindow.color

                Item {
                    height: parent.height
                    width: parent.width

                    MainToolBar {
                        anchors.left: parent.left
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom

                        color: dockWindow.color
                        currentUri: dockWindow.currentPageUri
                        onSelected: {
                            api.launcher.open(uri)
                        }
                    }

                    NotationToolBar {
                        anchors.centerIn: parent

                        visible: dockWindow.currentPageUri !== home.uri
                        color: dockWindow.color
                    }

                    PlaybackToolBar {
                        anchors.right: parent.right
                        anchors.top: parent.top
                        anchors.bottom: parent.bottom

                        visible: dockWindow.currentPageUri !== home.uri
                        color: dockWindow.color
                    }
                }
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
