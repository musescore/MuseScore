import QtQuick 2.7
import MuseScore.Ui 1.0
import MuseScore.Dock 1.0

import MuseScore.UiComponents 1.0
import MuseScore.Scores 1.0
import MuseScore.Extensions 1.0
import MuseScore.Cloud 1.0

DockPage {
    id: homePage

    objectName: "Home"

    AccountModel {
        id: accountModel
    }

    panels: [
        DockPanel {
            id: resourcesPanel
            objectName: "resourcesPanel"

            width: 200
            color: ui.theme.backgroundColor

            Column {
                anchors.fill: parent

                Rectangle {
                    height: 60
                    width: parent.width
                    color: ui.theme.backgroundColor

                    AccountInfoButton {
                        width: parent.width
                        anchors.verticalCenter: parent.verticalCenter

                        userName: accountModel.accountInfo.userName
                        avatarUrl: accountModel.accountInfo.avatarUrl

                        onClicked: {
                            homeCentral.load("account")
                        }
                    }
                }

                HomeMenu {
                    width: parent.width

                    onSelected: {
                        homeCentral.load(name)
                    }
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
            case "account":     currentComp = authorizationComp; break;
            }
        }

        Rectangle {
            Loader {
                id: centralLoader
                anchors.fill: parent
                sourceComponent: homeCentral.currentComp
            }

            Component.onCompleted: {
                accountModel.load()
            }
        }
    }

    Component {
        id: authorizationComp
        AuthorizationModule {
            onSignInRequested: {
                accountModel.signIn()
            }

            onCreateAccountRequested: {
                accountModel.createAccount()
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
}
