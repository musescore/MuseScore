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

import "internal"

FocusScope {
    id: root

    QtObject {
        id: prv

        readonly property int sideMargin: 46
    }

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

    Rectangle {
        id: background
        anchors.fill: parent
        color: ui.theme.backgroundSecondaryColor
    }

    CloudsModel {
        id: cloudsModel
    }

    Component.onCompleted: {
        cloudsModel.load()
    }

    StyledTextLabel {
        id: pageTitle

        anchors.top: parent.top
        anchors.topMargin: prv.sideMargin
        anchors.left: parent.left
        anchors.leftMargin: prv.sideMargin
        anchors.right: parent.right
        anchors.rightMargin: prv.sideMargin

        text: qsTrc("appshell", "Accounts")
        font: ui.theme.titleBoldFont
        horizontalAlignment: Text.AlignLeft
    }

    CloudsListView {
        id: view

        anchors.top: pageTitle.bottom
        anchors.topMargin: prv.sideMargin
        anchors.left: parent.left
        anchors.leftMargin: prv.sideMargin - spacingBetweenColumns / 2
        anchors.right: parent.right
        anchors.rightMargin: prv.sideMargin - spacingBetweenColumns / 2
        anchors.bottom: parent.bottom

        model: cloudsModel

        navigationSection: navSec

        onSignInRequested: function (cloudCode) {
            cloudsModel.signIn(cloudCode)
        }

        onSignOutRequested: function (cloudCode) {
            cloudsModel.signOut(cloudCode)
        }

        onCreateAccountRequested: function (cloudCode) {
            cloudsModel.createAccount(cloudCode)
        }
    }
}
