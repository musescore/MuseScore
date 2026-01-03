/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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

pragma ComponentBehavior: Bound

import QtQuick

import Muse.Ui
import Muse.UiComponents
import Muse.Cloud

StyledGridView {
    id: root

    cellWidth: actualCellWidth + spacingBetweenColumns
    cellHeight: actualCellHeight + spacingBetweenRows

    readonly property int columns: Math.max(0, Math.floor(width / cellWidth))

    readonly property real spacingBetweenColumns: 24
    readonly property real spacingBetweenRows: 24

    readonly property real actualCellWidth: 500
    readonly property real actualCellHeight: 200

    property NavigationSection navigationSection: null

    signal signInRequested(string cloudCode)
    signal signOutRequested(string cloudCode)
    signal createAccountRequested(string cloudCode)

    delegate: Item {
        id: item

        required property string cloudTitle
        required property bool userIsAuthorized
        required property string userName
        required property url userProfileUrl
        required property url userAvatarUrl
        required property url userCollectionUrl
        required property string cloudCode
        required property int index

        height: root.cellHeight
        width: root.cellWidth

        CloudItem {
            width: root.actualCellWidth
            height: root.actualCellHeight

            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter

            cloudTitle: item.cloudTitle
            userIsAuthorized: item.userIsAuthorized
            userName: item.userName
            userProfileUrl: item.userProfileUrl
            userAvatarUrl: item.userAvatarUrl
            userCollectionUrl: item.userCollectionUrl
            navigationPanel.section: root.navigationSection
            navigationPanel.order: item.index

            onSignInRequested: {
                root.signInRequested(item.cloudCode)
            }

            onSignOutRequested: {
                root.signOutRequested(item.cloudCode)
            }

            onCreateAccountRequested: {
                root.createAccountRequested(item.cloudCode)
            }
        }
    }
}
