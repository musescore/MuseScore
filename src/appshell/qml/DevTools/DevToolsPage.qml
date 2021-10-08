/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
import QtQuick 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Dock 1.0

import MuseScore.Plugins 1.0
import MuseScore.Audio 1.0

import "./Gallery"
import "./Interactive"
import "./NotationDialogs"
import "./Telemetry"
import "./VST"
import "./KeyNav"
import "./Preferences"

import "../dockwindow"

DockPage {
    id: root

    objectName: "DevTools"
    uri: "musescore://devtools"

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
        case "navigation": root.central = keynavComp; break
        }
    }

    panels: [
        DockPanel {
            id: devtoolsPanel

            objectName: "devtoolsPanel"

            minimumWidth: 200
            maximumWidth: 292

            allowedAreas: Qt.NoDockWidgetArea

            Rectangle {
                anchors.fill: parent
                color: ui.theme.backgroundPrimaryColor

                DevToolsMenu {
                    anchors.fill: parent

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
                        { "name": "navigation", "title": "KeyNav" }
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

        Playback {}
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
