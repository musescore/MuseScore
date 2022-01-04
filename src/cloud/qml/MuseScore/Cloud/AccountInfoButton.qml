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
import QtQuick 2.7

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Cloud 1.0

import "internal"

PageTabButton {
    id: root

    property string userName: accountModel.accountInfo.userName
    property string avatarUrl: accountModel.accountInfo.avatarUrl

    signal userAuthorizedChanged()

    orientation: Qt.Horizontal

    spacing: Boolean(avatarUrl) ? 18 : 30
    leftPadding: spacing

    title: Boolean(userName) ? userName : qsTrc("cloud", "My account")
    iconComponent: Boolean(avatarUrl) ? avatarComp : stubAvatarComp

    AccountModel {
        id: accountModel

        onUserAuthorizedChanged: {
            root.userAuthorizedChanged()
        }
    }

    Component.onCompleted: {
        accountModel.load()
    }

    Component {
        id: stubAvatarComp
        StyledIconLabel {
            anchors.verticalCenter: parent.verticalCenter
            height: root.height
            iconCode: IconCode.ACCOUNT
            visible: !Boolean(root.avatarUrl)
        }
    }

    Component {
        id: avatarComp
        AccountAvatar {
            url: root.avatarUrl
            side: 40
        }
    }
}
