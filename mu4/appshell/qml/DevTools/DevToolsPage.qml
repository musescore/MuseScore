import QtQuick 2.7
import MuseScore.Ui 1.0
import MuseScore.Dock 1.0
import MuseScore.UiComponents 1.0

import "./Interactive"
import "./Audio"


DockPage {
    id: homePage

    objectName: "devtools"

    panels: [
        DockPanel {
            id: resourcesPanel
            objectName: "devtoolsPanel"

            width: 200
            color: ui.theme.backgroundPrimaryColor

            DevToolsMenu {

                model: [
                    { "name": "interactive", "title": "Interactive" },
                    { "name": "audio", "title": "Audio" },
                    { "name": "synth", "title": "Synth" },
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

        property var currentComp: interactiveComp

        function load(name) {
            console.info("loadCentral: " + name)
            switch (name) {
            case "interactive": currentComp = interactiveComp; break
            case "audio": currentComp = audioComp; break
            case "synth": currentComp = synthSettingsComp; break
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
        id: interactiveComp
        InteractiveTests {}
    }

    Component{
        id: audioComp
        AudioEngineTests {}
    }

    Component {
        id: synthSettingsComp

        SynthSettings {}
    }
}
