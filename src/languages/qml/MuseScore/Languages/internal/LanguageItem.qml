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
import MuseScore.Languages 1.0

Rectangle {
    id: root

    property string title: ""
    property string statusTitle: ""

    property real headerWidth: width / 2
    property real sideMargin: 0.0

    property alias navigation: navCtrl

    signal clicked()
    signal navigationActive()

    height: 48

    color: ui.theme.backgroundPrimaryColor

    border.width: navCtrl.active ? 2 : 0
    border.color: ui.theme.focusColor

    function ensureActiveFocus() {
        if (!root.activeFocus) {
            root.forceActiveFocus()
        }
    }

    NavigationControl {
        id: navCtrl
        name: root.title
        enabled: root.enabled && root.visible
        accessible.role: MUAccessible.ListItem
        accessible.name: root.title + " " + root.statusTitle

        onActiveChanged: {
            if (navCtrl.active) {
                root.navigationActive()
            }
        }

        onTriggered: root.clicked()
    }

    Row {
        anchors.left: parent.left
        anchors.leftMargin: sideMargin
        anchors.right: parent.right
        anchors.rightMargin: sideMargin

        anchors.verticalCenter: parent.verticalCenter

        Row {
            width: headerWidth

            spacing: 12

            StyledIconLabel {
                iconCode: IconCode.NEW_FILE
            }

            StyledTextLabel {
                text: title
                font: ui.theme.largeBodyFont
                horizontalAlignment: Text.AlignLeft
            }
        }

        StyledTextLabel {
            width: headerWidth

            text: statusTitle
            font: ui.theme.largeBodyFont
            horizontalAlignment: Text.AlignLeft
        }
    }

    MouseArea {
        anchors.fill: parent

        onClicked: {
            root.ensureActiveFocus()
            root.clicked()
        }
    }
}
