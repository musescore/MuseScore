import QtQuick 2.7
import MuseScore.Ui 1.0
import MuseScore.Dock 1.0
import MuseScore.UiComponents 1.0

import "./Interactive"
import "./Audio"
import "./NotationDialogs"
import "./VST"
import "./Plugins"

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
                    { "name": "midiports", "title": "Midi ports" },
                    { "name": "mu3dialogs", "title": "MU3Dialogs" },
                    { "name": "vst", "title": "VST" },
                    { "name": "plugins", "title": "Plugins" },
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
            case "midiports": currentComp = midiPortsComp; break
            case "mu3dialogs": currentComp = notationDialogs; break
            case "vst": currentComp = vstComponent; break
            case "plugins": currentComp = pluginsComp; break
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

    Component {
        id: midiPortsComp
        MidiPorts {}
    }

    Component {
        id: notationDialogs
        MU3Dialogs {}
    }

    Component {
        id: vstComponent
        //safe if VST is not available
        Loader {
            source: "qrc:/qml/DevTools/VST/VSTTests.qml"
        }
    }

    Component {
        id: pluginsComp
        PluginsTests {}
    }
}
