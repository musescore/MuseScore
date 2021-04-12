import QtQuick 2.15
import QtQuick.Controls 2.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

import MuseScore.UserScores 1.0
import MuseScore.Cloud 1.0

import "../docksystem"
import "../../HomePage"

DockPage {
    id: root

    property string item: "scores"
    property string subItem: ""

    uniqueName: "Home"

    onItemChanged: {
        Qt.callLater(root.setCurrentCentral, item)
    }

    function setCurrentCentral(name) {
        if (item === name || !Boolean(name)) {
            return
        }

        item = name

        switch (name) {
        case "scores": root.central = scoresComp; break
        case "add-ons": root.central = addonsComp; break
        case "audio": root.central = audioComp; break
        case "feautured": root.central = feauturedComp; break
        case "learn": root.central = learnComp; break
        case "support": root.central = supportComp; break
        case "account": root.central = accountComp; break
        }
    }

    panels: [
        DockPanel {
            uniqueName: "homeMenu"

            width: 292
            minimumWidth: 76

            HomeMenu {
                currentPageName: root.item

                onSelected: {
                    root.setCurrentCentral(name)
                }
            }
        }
    ]

    central: scoresComp

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
            item: root.subItem
        }
    }

    Component {
        id: audioComp

        StyledTextLabel {
            anchors.centerIn: parent
            text: "Audio & VST"
        }
    }

    Component {
        id: feauturedComp

        StyledTextLabel {
            anchors.centerIn: parent
            text: "Feautured"
        }
    }

    Component {
        id: learnComp

        StyledTextLabel {
            anchors.centerIn: parent
            text: "Learn"
        }
    }

    Component {
        id: supportComp

        StyledTextLabel {
            anchors.centerIn: parent
            text: "Support"
        }
    }
}
