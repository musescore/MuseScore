import QtQuick 2.7
import MuseScore.Ui 1.0
import MuseScore.Dock 1.0
import MuseScore.UiComponents 1.0

import "./Launcher"


DockPage {
    id: homePage

    objectName: "devtools"

    panels: [
        DockPanel {
            id: resourcesPanel
            objectName: "devtoolsPanel"

            width: 200
            color: ui.theme.window

            DevToolsMenu {

                model: [
                    { "name": "launcher", "title": "Launcher" },
                    { "name": "sample", "title": "Sample" },
                ]

                onSelected: {
                    devtoolsCentral.load(name)
                }

            }
        }
    ]

    central: DockCentral {
        id: devtoolsCentral
        objectName: "devtoolsCentral"

        property var currentComp: launcherComp

        function load(name) {
            console.info("loadCentral: " + name)
            switch (name) {
            case "launcher":      currentComp = launcherComp; break
            case "sample":        currentComp = sampleComp; break
            }
        }

        Rectangle {

            Loader {
                id: centralLoader
                anchors.fill: parent
                sourceComponent: devtoolsCentral.currentComp
            }
        }
    }

    Component {
        id: launcherComp
        LauncherTests {}
    }

    Component {
        id: sampleComp

        Rectangle {
            Text {
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                text: "Sample"
            }
        }
    }
}
