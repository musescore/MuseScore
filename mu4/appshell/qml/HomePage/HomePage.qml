import QtQuick 2.9
import MuseScore.Ui 1.0
import MuseScore.Dock 1.0

import MuseScore.UiComponents 1.0
import MuseScore.Scores 1.0
import MuseScore.Extensions 1.0

DockPage {
    id: homePage

    objectName: "home"

    panels: [
        DockPanel {
            id: resourcesPanel
            objectName: "resourcesPanel"

            width: 200
            color: ui.theme.window

            HomeMenu {

                model: [
                    { "name": "scores", "title": "Scores" },
                    { "name": "extensions", "title": "Add-ons" },
                    { "name": "audio", "title": "Audio & VST" },
                    { "name": "feautured", "title": "Featured" },
                    { "name": "learn", "title": "Learn" },
                    { "name": "support", "title": "Support" },
                    { "name": "account", "title": "Account" }
                ]

                onSelected: {
                    homeCentral.load(name)
                }

            }
        }
    ]

    central: DockCentral {
        id: homeCentral
        objectName: "homeCentral"

        property var currentComp: scoresComp

        function load(name) {
            console.info("loadCentral: " + name)
            switch (name) {
            case "scores":      currentComp = scoresComp; break
            case "extensions":  currentComp = extensionsComp; break
            case "audio":       currentComp = audioComp; break
            case "feautured":   currentComp = feauturedComp; break
            case "learn":       currentComp = learnComp; break
            case "support":     currentComp = supportComp; break
            case "account":     currentComp = accountComp; break
            }
        }

        Rectangle {

            Loader {
                id: centralLoader
                anchors.fill: parent
                sourceComponent: homeCentral.currentComp
            }
        }
    }

    Component {
        id: scoresComp
        ScoresModule {}
    }

    Component {
        id: extensionsComp
        ExtensionsModule {}
    }

    Component {
        id: audioComp

        Rectangle {
            Text {
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                text: "Audio & VST"
            }
        }
    }

    Component {
        id: feauturedComp

        Rectangle {
            Text {
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                text: "Feautured"
            }
        }
    }

    Component {
        id: learnComp

        Rectangle {
            Text {
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                text: "Learn"
            }
        }
    }

    Component {
        id: supportComp

        Rectangle {
            Text {
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                text: "Support"
            }
        }
    }

    Component {
        id: accountComp

        Rectangle {
            Text {
                anchors.fill: parent
                verticalAlignment: Text.AlignVCenter
                horizontalAlignment: Text.AlignHCenter
                text: "Account"
            }
        }
    }
}
