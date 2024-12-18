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

import Muse.Ui 1.0
import Muse.UiComponents 1.0

import ".."

Item {
    id: root

    property string cloudTitle: ""
    property bool userIsAuthorized: false
    property string userName: ""
    property url userProfileUrl
    property url userAvatarUrl
    property url userCollectionUrl

    property NavigationPanel navigationPanel: NavigationPanel {
        name: root.cloudTitle + "Item"
        direction: NavigationPanel.Both

        onActiveChanged: function(active) {
            if (active) {
                firstButton.navigation.requestActive()
                accessibleInfo.ignored = false
                accessibleInfo.focused = true
            } else {
                accessibleInfo.ignored = true
                accessibleInfo.focused = false
                firstButton.accessible.ignored = true
            }
        }
    }

    signal signInRequested()
    signal signOutRequested()
    signal createAccountRequested()

    AccessibleItem {
        id: accessibleInfo
        accessibleParent: root.navigationPanel.accessible
        visualItem: root
        role: MUAccessible.Button
        name: {
            var msg = ""
            if (Boolean(root.userIsAuthorized)) {
                msg = "%1. %2. %3. %4".arg(root.cloudTitle)
                .arg(root.userName)
                .arg(root.userCollectionUrl)
                .arg(firstButton.text)
            } else {
                msg = "%1. %2. %3".arg(root.cloudTitle)
                .arg(qsTrc("cloud", "Not signed in"))
                .arg(firstButton.text)
            }

            return msg
        }
    }

    StyledTextLabel {
        id: cloudTitleLabel

        anchors.left: parent.left
        anchors.right: parent.right

        text: root.cloudTitle

        font: ui.theme.tabBoldFont
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

                    text: Boolean(root.userIsAuthorized) ? root.userName : qsTrc("cloud", "Not signed in")

                    font: ui.theme.headerBoldFont
                    horizontalAlignment: Text.AlignLeft
                }

                StyledTextLabel {
                    anchors.left: parent.left
                    anchors.right: parent.right

                    text: Boolean(root.userIsAuthorized) ? root.userCollectionUrl : root.cloudTitle

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
                    id: firstButton

                    width: (parent.width - parent.spacing) / 2

                    text: Boolean(root.userIsAuthorized) ? qsTrc("cloud", "My profile") : qsTrc("cloud", "Sign in")
                    accentButton: true

                    navigation.panel: root.navigationPanel
                    navigation.name: "FirstButton"
                    navigation.order: 1
                    navigation.accessible.ignored: true
                    navigation.onActiveChanged: {
                        if (!navigation.active) {
                            accessible.ignored = false
                            accessibleInfo.ignored = true
                        }
                    }

                    onClicked: {
                        if (Boolean(root.userIsAuthorized)) {
                            api.launcher.openUrl(root.userProfileUrl)
                        } else {
                            signInRequested()
                        }
                    }
                }

                FlatButton {
                    id: secondButton

                    width: (parent.width - parent.spacing) / 2

                    text: Boolean(root.userIsAuthorized) ? qsTrc("cloud", "Sign out") : qsTrc("cloud", "Create account")
                    accentButton: !Boolean(root.userIsAuthorized)

                    navigation.panel: root.navigationPanel
                    navigation.name: "SecondButton"
                    navigation.order: 2

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
