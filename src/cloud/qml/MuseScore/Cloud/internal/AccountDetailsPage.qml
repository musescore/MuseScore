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

import MuseScore.UiComponents 1.0
import MuseScore.Cloud 1.0

FocusScope {
    id: root

    property alias userName: userName.text
    property alias avatarUrl: accountAvatar.url
    property string profileUrl: ""
    property string sheetmusicUrl: ""

    signal signOutRequested()

    QtObject {
        id: prv

        readonly property int sideMargin: 46
        readonly property int buttonWidth: 133
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

    Row {
        anchors.top: userName.bottom
        anchors.topMargin: 106
        anchors.left: parent.left
        anchors.leftMargin: prv.sideMargin

        width: parent.width
        spacing: 67

        AccountAvatar {
            id: accountAvatar

            side: 200
        }

        Column {
            anchors.verticalCenter: parent.verticalCenter
            spacing: 20

            StyledTextLabel {
                text: qsTrc("cloud", "Your profile link:")
                font: ui.theme.largeBodyFont
            }

            StyledTextLabel {
                text: "MuseScore.com/" + root.userName
                font: ui.theme.largeBodyBoldFont

                MouseArea {
                    anchors.fill: parent

                    onClicked: {
                        api.launcher.openUrl(root.sheetmusicUrl)
                    }
                }
            }
        }
    }

    Rectangle {
        anchors.bottom: parent.bottom

        height: 114
        width: parent.width

        color: ui.theme.popupBackgroundColor

        Row {
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: prv.sideMargin

            spacing: 22

            FlatButton {
                width: prv.buttonWidth
                text: qsTrc("cloud", "Account info")

                accentButton: true

                onClicked: {
                    api.launcher.openUrl(root.profileUrl)
                }
            }

            FlatButton {
                width: prv.buttonWidth
                text: qsTrc("cloud", "Sign out")

                onClicked: {
                    root.signOutRequested()
                }
            }
        }
    }
}
