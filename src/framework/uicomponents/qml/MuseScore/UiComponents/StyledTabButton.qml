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
import QtQuick.Controls 2.2

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0

TabButton {
    id: root

    property int sideMargin: 0
    property bool isCurrent: false
    property string backgroundColor: ui.theme.backgroundPrimaryColor

    property alias navigation: navCtrl

    signal navigationTriggered()

    width: implicitWidth + sideMargin * 2 - 8

    onIsCurrentChanged: {
        if (root.isCurrent) {
            root.ensureActiveFocus()
        }
    }

    onPressed: {
        root.ensureActiveFocus()
    }

    function ensureActiveFocus() {
        if (!root.activeFocus) {
            root.forceActiveFocus()
        }
    }

    NavigationControl {
        id: navCtrl
        name: root.objectName != "" ? root.objectName : "TabButton"

        accessible.role: MUAccessible.RadioButton
        accessible.name: root.text
        accessible.checked: root.isCurrent

        onActiveChanged: {
            if (active) {
                root.ensureActiveFocus()
            }
        }
        onTriggered: root.navigationTriggered()
    }

    contentItem: StyledTextLabel {
        id: textLabel
        text: root.text
        font: ui.theme.largeBodyFont
        opacity: 0.75
    }

    background: Rectangle {
        implicitHeight: 32

        color: root.backgroundColor

        NavigationFocusBorder { navigationCtrl: navCtrl }

        border.width: ui.theme.borderWidth
        border.color: ui.theme.strokeColor

        Rectangle {
            id: selectedRect

            anchors.left: parent.left
            anchors.leftMargin: sideMargin
            anchors.right: parent.right
            anchors.rightMargin: sideMargin
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 4

            height: 2

            visible: isCurrent
            color: ui.theme.accentColor
        }
    }

    states: [
        State {
            name: "HOVERED"
            when: root.hovered && !isCurrent

            PropertyChanges {
                target: contentItem
                opacity: 1
            }
        },

        State {
            name: "SELECTED"
            when: isCurrent

            PropertyChanges {
                target: textLabel
                opacity: 1
                font: ui.theme.largeBodyBoldFont
            }
        }
    ]
}

