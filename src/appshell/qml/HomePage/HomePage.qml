import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2

import MuseScore.Ui 1.0
import MuseScore.Dock 1.0
import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0
import MuseScore.Cloud 1.0

DockPage {
    id: homePage

    objectName: "Home"

    property string item: ""
    property string subItem: ""

    onItemChanged: {
        if (!Boolean(item)) {
            return
        }

        if (homeCentral.currentCompName !== item) {
            Qt.callLater(homeCentral.load, item)
        }
    }

    panels: [
        DockPanel {
            id: homePanel
            objectName: "homePanel"

            width: 292
            minimumWidth: 76

            color: ui.theme.backgroundPrimaryColor

             HomeMenu {
                currentPageName: homeCentral.currentCompName

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
        property var currentCompName: "scores"

        function load(name) {
            console.info("loadCentral: " + name)
            currentCompName = name
            switch (name) {
            case "scores":      currentComp = scoresComp; break
            case "add-ons":     currentComp = addonsComp; break
            case "audio":       currentComp = audioComp; break
            case "feautured":   currentComp = feauturedComp; break
            case "learn":       currentComp = learnComp; break
            case "support":     currentComp = supportComp; break
            case "account":     currentComp = accountComp; break;
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
        id: accountComp

        AccountPage {}
    }

    Component {
        id: scoresComp
        ScoresPage {}
    }

    Component {
        id: addonsComp
        AddonsContent {
            item: homePage.subItem
        }
    }

    Component {
        id: audioComp

        Rectangle {
            StyledTextLabel {
                anchors.fill: parent
                text: "Audio & VST"
            }
        }
    }

    Component {
        id: feauturedComp

        Rectangle {
            StyledTextLabel {
                anchors.fill: parent
                text: "Feautured"
            }
        }
    }

    Component {
        id: learnComp

        Rectangle {
            StyledTextLabel {
                anchors.fill: parent
                text: "Learn"
            }
        }
    }

    Component {
        id: supportComp

        Rectangle {
            StyledTextLabel {
                anchors.fill: parent
                text: "Support"
            }
        }
    }
}
