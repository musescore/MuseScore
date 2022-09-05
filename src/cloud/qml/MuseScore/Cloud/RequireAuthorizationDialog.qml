/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2022 MuseScore BVBA and others
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
import MuseScore.Cloud 1.0

StyledDialogView {
    id: root

    property alias text: content.text

    contentWidth: content.implicitWidth
    contentHeight: content.implicitHeight

    margins: 16

    AccountModel {
        id: accountModel

        onUserAuthorizedChanged: {
            if (userAuthorized) {
                root.accept()
            }
        }
    }

    Component.onCompleted: {
        accountModel.load()
    }

    enum ButtonId {
        Cancel,
        CreateAccount,
        Login
    }

    StandardDialogPanel {
        id: content
        anchors.fill: parent

        navigation.section: root.navigationSection

        title: qsTrc("cloud", "You are not signed in")

        buttons: [
            { "buttonId": RequireAuthorizationDialog.Cancel, "title": qsTrc("global", "Cancel") },
            { "buttonId": RequireAuthorizationDialog.CreateAccount, "title": qsTrc("cloud", "Create account") },
            { "buttonId": RequireAuthorizationDialog.Login, "title": qsTrc("cloud", "Login") }
        ]

        onClicked: function(buttonId, showAgain) {
            switch (buttonId) {
            case RequireAuthorizationDialog.Cancel:
                root.hide()
                return
            case RequireAuthorizationDialog.CreateAccount:
                accountModel.createAccount()
                return
            case RequireAuthorizationDialog.Login:
                accountModel.signIn()
                return
            }
        }
    }
}

