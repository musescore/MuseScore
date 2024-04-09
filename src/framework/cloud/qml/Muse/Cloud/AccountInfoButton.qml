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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.Cloud 1.0

import "internal"

PageTabButton {
    id: root

    property var cloudInfo: Boolean(cloudsModel.userAuthorized) ? cloudsModel.firstAuthorizedCloudInfo() : null

    signal userAuthorizedChanged()

    orientation: Qt.Horizontal

    spacing: 22
    leftPadding: spacing

    title: Boolean(root.cloudInfo) ? root.cloudInfo.userName : qsTrc("cloud", "My accounts")
    iconComponent: AccountAvatar {
        url: Boolean(root.cloudInfo) ? root.cloudInfo.userAvatarUrl : null
        side: 32

        withBackground: false
    }

    CloudsModel {
        id: cloudsModel

        onUserAuthorizedChanged: {
            root.userAuthorizedChanged()
        }
    }

    Component.onCompleted: {
        cloudsModel.load()
    }
}
