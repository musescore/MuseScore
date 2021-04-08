import QtQuick 2.15
import QtQuick.Controls 2.15

import MuseScore.AppShell 1.0
import MuseScore.Ui 1.0
import MuseScore.Dock 1.0
import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0
import MuseScore.Cloud 1.0

import "./docksystem"
import "./HomePage"
import "./NotationPage"
import "./DevTools"

import com.kdab.dockwidgets 1.0 as KDDW

ApplicationWindow {
    id: root

    width: 800
    height: 600

    visible: true

    title: qsTrc("appshell", "MuseScore 4")

    DockPage {
        uri: "musescore://home/scores"
        uniqueName: "scores"

        panels: [
            DockPanel {
                uniqueName: "homeMenu"

                HomeMenu {}
            }
        ]

        central: ScoresPage {}
    }
}
