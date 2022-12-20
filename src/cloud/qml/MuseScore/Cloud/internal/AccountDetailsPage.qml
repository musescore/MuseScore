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
import MuseScore.UiComponents 1.0
import MuseScore.Cloud 1.0

FocusScope {
    id: root

    property alias userName: userName.text
    property alias avatarUrl: accountInfo.url
    property string profileUrl: ""
    property string sheetmusicUrl: ""

    property NavigationSection navigationSection: null

    signal signOutRequested()

    QtObject {
        id: prv

        readonly property int sideMargin: 46
        readonly property int buttonWidth: 133
    }

    NavigationPanel {
        id: navPanel
        name: "AccountDetailsPanel"
        direction: NavigationPanel.Horizontal
        section: root.navigationSection
        accessible.name: userName.text

        onActiveChanged: function(active) {
            if (active) {
                accountInfoButton.navigation.requestActive()
                accountInfo.readInfo()
            } else {
                accountInfo.resetFocusOnInfo()
                accountInfoButton.accessible.ignored = true
            }
        }
    }

    Rectangle {
        anchors.fill: parent
        color: ui.theme.backgroundSecondaryColor
    }

    StyledTextLabel {
        id: userName

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.topMargin: prv.sideMargin
        anchors.leftMargin: prv.sideMargin

        font: ui.theme.titleBoldFont
    }

    AccountInfo {
        id: accountInfo
        anchors.left: parent.left
        anchors.leftMargin: prv.sideMargin
        anchors.top: userName.bottom
        anchors.topMargin: 106

        userName: root.userName
        sheetmusicUrl: root.sheetmusicUrl

        navigationPanel: navPanel
        activeButtonName: accountInfoButton.text
    }

    Rectangle {
        anchors.bottom: parent.bottom

        height: 100
        width: parent.width

        color: ui.theme.backgroundSecondaryColor

        Row {
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: prv.sideMargin

            spacing: 22

            FlatButton {
                id: accountInfoButton
                minWidth: prv.buttonWidth
                text: qsTrc("cloud", "Account info")

                accentButton: true

                navigation.name: "AccountInfo"
                navigation.panel: navPanel
                navigation.column: 1
                navigation.accessible.ignored: true
                navigation.onActiveChanged: {
                    if (!navigation.active) {
                        accessible.ignored = false
                        accountInfo.resetFocusOnInfo()
                    }
                }

                onClicked: {
                    api.launcher.openUrl(root.profileUrl)
                }
            }

            FlatButton {
                minWidth: prv.buttonWidth
                text: qsTrc("cloud", "Sign out")

                navigation.name: "SignOut"
                navigation.panel: navPanel
                navigation.column: 2

                onClicked: {
                    root.signOutRequested()
                }
            }
        }
    }
}
