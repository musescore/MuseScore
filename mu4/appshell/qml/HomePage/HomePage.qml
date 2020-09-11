import QtQuick 2.7
import QtQuick.Layouts 1.3

import MuseScore.Ui 1.0
import MuseScore.Dock 1.0
import MuseScore.UiComponents 1.0
import MuseScore.UserScores 1.0
import MuseScore.Extensions 1.0
import MuseScore.Cloud 1.0

DockPage {
    id: homePage

    objectName: "Home"

    AccountModel {
        id: accountModel

        onUserAuthorizedChanged: {
            homeCentral.load("scores")
        }
    }

    panels: [
        DockPanel {
            id: resourcesPanel
            objectName: "resourcesPanel"

            width: 200
            color: ui.theme.backgroundPrimaryColor

            ColumnLayout {
                anchors.fill: parent

                spacing: 0

                Rectangle {
                    Layout.preferredHeight: 60
                    Layout.fillWidth: true

                    color: ui.theme.backgroundPrimaryColor

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
                    Layout.fillWidth: true

                    onSelected: {
                        homeCentral.load(name)
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.fillHeight: true

                    color: ui.theme.backgroundPrimaryColor
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
            case "add-ons":     currentComp = addonsComp; break
            case "audio":       currentComp = audioComp; break
            case "feautured":   currentComp = feauturedComp; break
            case "learn":       currentComp = learnComp; break
            case "support":     currentComp = supportComp; break
            case "account":     currentComp = accountModel.userAuthorized ? accountDetailsComp : authorizationComp; break;
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

        AuthorizationContent {
            onSignInRequested: {
                accountModel.signIn()
            }

            onCreateAccountRequested: {
                accountModel.createAccount()
            }
        }
    }

    Component {
        id: accountDetailsComp

        AccountDetailsContent {
            userName: accountModel.accountInfo.userName
            avatarUrl: accountModel.accountInfo.avatarUrl
            profileUrl: accountModel.accountInfo.profileUrl
            sheetmusicUrl: accountModel.accountInfo.sheetmusicUrl

            onSignOutRequested: {
                accountModel.signOut()
            }
        }
    }

    Component {
        id: scoresComp
        UserScoresContent {}
    }

    Component {
        id: addonsComp
        AddonsContent {}
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
