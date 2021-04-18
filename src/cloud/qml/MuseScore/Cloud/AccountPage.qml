import QtQuick 2.15

import MuseScore.Cloud 1.0

import "internal"

FocusScope {
    id: root

    AccountModel {
        id: accountModel

        onUserAuthorizedChanged: {
            root.userAuthorizedChanged()
        }
    }

    Component.onCompleted: {
        accountModel.load()
    }

    Loader {
        anchors.fill: parent

        sourceComponent: accountModel.userAuthorized ? authorizationComp : accountDetailsComp
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
}
