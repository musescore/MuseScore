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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.Cloud 1.0

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

        height: root.cellHeight
        width: root.cellWidth

        CloudItem {
            width: root.actualCellWidth
            height: root.actualCellHeight

            anchors.top: parent.top
            anchors.horizontalCenter: parent.horizontalCenter

            cloudTitle: model.cloudTitle
            userIsAuthorized: model.userIsAuthorized
            userName: model.userName
            userProfileUrl: model.userProfileUrl
            userAvatarUrl: model.userAvatarUrl
            userCollectionUrl: model.userCollectionUrl

            navigationPanel.section: root.navigationSection
            navigationPanel.order: model.index

            onSignInRequested: {
                root.signInRequested(model.cloudCode)
            }

            onSignOutRequested: {
                root.signOutRequested(model.cloudCode)
            }

            onCreateAccountRequested: {
                root.createAccountRequested(model.cloudCode)
            }
        }
    }
}
