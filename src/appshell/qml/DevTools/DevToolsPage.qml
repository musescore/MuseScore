import QtQuick 2.15

import MuseScore.Dock 1.0
import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Plugins 1.0
import MuseScore.Audio 1.0

import "./Preferences"
import "./Gallery"
import "./Interactive"
import "./NotationDialogs"
import "./Telemetry"
import "./VST"
import "./KeyNav"

DockPage {
    id: homePage

    objectName: "devtools"

    panels: [
        DockPanel {
            id: devtoolsPanel
            objectName: "devtoolsPanel"

            width: 292
            minimumWidth: 200

            color: ui.theme.backgroundPrimaryColor

            Rectangle {
                anchors.fill: parent
                color: ui.theme.backgroundPrimaryColor

                DevToolsMenu {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right

                    model: [
                        { "name": "settings", "title": "Settings" },
                        { "name": "gallery", "title": "UI Gallery" },
                        { "name": "interactive", "title": "Interactive" },
                        { "name": "mu3dialogs", "title": "MU3Dialogs" },
                        { "name": "telemetry", "title": "Telemetry" },
                        { "name": "audio", "title": "Audio" },
                        { "name": "synth", "title": "Synth" },
                        { "name": "midiports", "title": "MIDI ports" },
                        { "name": "vst", "title": "VST" },
                        { "name": "plugins", "title": "Plugins" },
                        { "name": "autobot", "title": "Autobot" },
                        { "name": "keynav", "title": "KeyNav" }
                    ]

                    onSelected: {
                        devtoolsCentral.load(name)
                    }
                }
            }
        }
    ]

    central: DockCentral {
        id: devtoolsCentral
        objectName: "devtoolsCentral"

        property var currentComp: settingsComp

        function load(name) {
            console.info("loadCentral: " + name)
            switch (name) {
            case "settings": currentComp = settingsComp; break
            case "gallery": currentComp = galleryComp; break
            case "interactive": currentComp = interactiveComp; break
            case "mu3dialogs": currentComp = notationDialogs; break
            case "telemetry": currentComp = telemetryComp; break
            case "audio": currentComp = audioComp; break
            case "synth": currentComp = synthSettingsComp; break
            case "midiports": currentComp = midiPortsComp; break
            case "vst": currentComp = vstComponent; break
            case "plugins": currentComp = pluginsComp; break
            case "autobot": currentComp = autobotComp; break
            case "keynav": currentComp = keynavComp; break
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
        id: settingsComp
        SettingsPage {}
    }

    Component {
        id: galleryComp
        GeneralComponentsGallery {}
    }

    Component {
        id: interactiveComp
        InteractiveTests {}
    }

    Component {
        id: notationDialogs
        MU3Dialogs {}
    }

    Component {
        id: telemetryComp
        Loader {
            source: "qrc:/qml/DevTools/Telemetry/TelemetryInfo.qml"
        }
    }

    Component {
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

    Component {
        id: autobotComp
        Loader {
            source: "qrc:/qml/DevTools/Autobot/AutobotControl.qml"
        }
    }

    Component {
        id: keynavComp
        KeyNavExample{}
    }

}
