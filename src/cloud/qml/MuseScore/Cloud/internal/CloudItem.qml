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
import QtQuick.Layouts 1.15

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

import ".."

Item {
    id: root

    property string cloudTitle: ""
    property bool userIsAuthorized: false
    property string userName: ""
    property var userProfileUrl: null
    property var userAvatarUrl: null
    property var userCollectionUrl: null

    signal signInRequested()
    signal signOutRequested()
    signal createAccountRequested()

    property NavigationControl navigation: NavigationControl {
        accessible.role: MUAccessible.ListItem
        //        accessible.name: root.name
        enabled: root.enabled && root.visible

        onActiveChanged: function(active) {
            if (active) {
                root.forceActiveFocus()
            }
        }

        onTriggered: root.clicked()
    }

    StyledTextLabel {
        id: cloudTitleLabel

        anchors.left: parent.left
        anchors.right: parent.right

        text: root.cloudTitle

        font: ui.theme.headerFont
        horizontalAlignment: Text.AlignLeft
    }

    Rectangle {
        anchors.top: cloudTitleLabel.bottom
        anchors.topMargin: 20
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom

        color: ui.theme.backgroundPrimaryColor

        radius: 12
        border.width: 1
        border.color: ui.theme.strokeColor

        Item {
            anchors.fill: parent
            anchors.margins: 24

            AccountAvatar {
                id: avatar

                url: root.userAvatarUrl
                side: 100
            }

            Column {
                anchors.top: parent.top
                anchors.left: avatar.right
                anchors.leftMargin: 24
                anchors.right: parent.right

                spacing: 12

                StyledTextLabel {
                    anchors.left: parent.left
                    anchors.right: parent.right

                    text: root.userName

                    font: ui.theme.headerBoldFont
                    horizontalAlignment: Text.AlignLeft
                }

                StyledTextLabel {
                    anchors.left: parent.left
                    anchors.right: parent.right

                    text: root.userCollectionUrl

                    font: ui.theme.tabFont
                    horizontalAlignment: Text.AlignLeft
                }
            }

            Row {
                spacing: 12

                anchors.left: avatar.right
                anchors.leftMargin: 24
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                FlatButton {
                    width: (parent.width - parent.spacing) / 2

                    text: Boolean(root.userIsAuthorized) ? qsTrc("cloud", "My profile") : qsTrc("cloud", "Sign in")
                    accentButton: true

                    onClicked: {
                        if (Boolean(root.userIsAuthorized)) {
                            api.launcher.openUrl(root.userProfileUrl)
                        } else {
                            signInRequested()
                        }
                    }
                }

                FlatButton {
                    width: (parent.width - parent.spacing) / 2

                    text: Boolean(root.userIsAuthorized) ? qsTrc("cloud", "Sign out") : qsTrc("cloud", "Create account")
                    accentButton: !Boolean(root.userIsAuthorized)

                    onClicked: {
                        if (Boolean(root.userIsAuthorized)) {
                            signOutRequested()
                        } else {
                            createAccountRequested()
                        }
                    }
                }
            }
        }
    }
}
