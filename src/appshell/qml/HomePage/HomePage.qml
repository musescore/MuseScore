import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.2

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

            width: 292
            minimumWidth: 76

            color: ui.theme.backgroundPrimaryColor

            Rectangle {
                anchors.fill: parent
                color: ui.theme.backgroundPrimaryColor

                ColumnLayout {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right

                    spacing: 0

                    AccountInfoButton {
                        Layout.preferredHeight: 60
                        Layout.fillWidth: true

                        ButtonGroup.group: homeMenuButtons.radioButtonGroup

                        userName: accountModel.accountInfo.userName
                        avatarUrl: accountModel.accountInfo.avatarUrl

                        checked: homeCentral.currentCompName == "account"

                        onToggled: {
                            homeCentral.load("account")
                        }
                    }

                    HomeMenu {
                        id: homeMenuButtons
                        Layout.topMargin: 20
                        Layout.fillWidth: true

                        onSelected: {
                            homeCentral.load(name)
                        }
                    }
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

        AuthorizationPage {
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

        AccountDetailsPage {
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
        ScoresPage {}
    }

    Component {
        id: addonsComp
        AddonsContent {}
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
