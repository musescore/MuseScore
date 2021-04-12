import QtQuick 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Plugins 1.0
import MuseScore.Audio 1.0

import "../../DevTools"
import "../../DevTools/Gallery"
import "../../DevTools/Interactive"
import "../../DevTools/NotationDialogs"
import "../../DevTools/Telemetry"
import "../../DevTools/VST"
import "../../DevTools/KeyNav"
import "../../DevTools/Preferences"

import "../docksystem"

DockPage {
    id: root

    uniqueName: "DevTools"

    function setCurrentCentral(name) {
        switch (name) {
        case "settings": root.central = settingsComp; break
        case "gallery": root.central = galleryComp; break
        case "interactive": root.central = interactiveComp; break
        case "mu3dialogs": root.central = notationDialogs; break
        case "telemetry": root.central = telemetryComp; break
        case "audio": root.central = audioComp; break
        case "synth": root.central = synthSettingsComp; break
        case "midiports": root.central = midiPortsComp; break
        case "vst": root.central = vstComponent; break
        case "plugins": root.central = pluginsComp; break
        case "autobot": root.central = autobotComp; break
        case "keynav": root.central = keynavComp; break
        }
    }

    panels: [
        DockPanel {
            id: devtoolsPanel

            uniqueName: "devtoolsPanel"

            width: 292
            minimumWidth: 200

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
                        root.setCurrentCentral(name)
                    }
                }
            }
        }
    ]

    central: settingsComp

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

        KeyNavExample {}
    }
}
