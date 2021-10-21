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

Row {
    id: root

    property alias url: accountAvatar.url
    property string userName: ""
    property string sheetmusicUrl: ""

    property NavigationPanel navigationPanel: null
    property string activeButtonName: ""

    width: parent.width
    spacing: 67

    function readInfo() {
        accessibleInfo.focused = true
    }

    function resetFocusOnInfo() {
        accessibleInfo.focused = false
    }

    AccessibleItem {
        id: accessibleInfo
        accessibleParent: root.navigationPanel.accessible
        visualItem: root
        role: MUAccessible.Information
        name: root.userName + ". " + profileLinkLabel.text + " " + profileLink.text + ". " + root.activeButtonName
    }

    AccountAvatar {
        id: accountAvatar

        side: 200
    }

    Column {
        anchors.verticalCenter: parent.verticalCenter
        spacing: 20

        StyledTextLabel {
            id: profileLinkLabel
            text: qsTrc("cloud", "Your profile link:")
            font: ui.theme.largeBodyFont
        }

        StyledTextLabel {
            id: profileLink
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
