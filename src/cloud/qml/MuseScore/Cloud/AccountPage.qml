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
import MuseScore.Cloud 1.0

import "internal"

FocusScope {
    id: root

    NavigationSection {
        id: navSec
        name: "Account"
        enabled: root.enabled && root.visible
        order: 3
        onActiveChanged: function(active) {
            if (active) {
                root.forceActiveFocus()
            }
        }
    }

    AccountModel {
        id: accountModel
    }

    Component.onCompleted: {
        accountModel.load()
    }

    Loader {
        anchors.fill: parent

        sourceComponent: accountModel.userAuthorized ? accountDetailsComp : authorizationComp
    }

    Component {
        id: authorizationComp

        AuthorizationPage {
            navigationSection: navSec

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

            navigationSection: navSec

            onSignOutRequested: {
                accountModel.signOut()
            }
        }
    }
}
