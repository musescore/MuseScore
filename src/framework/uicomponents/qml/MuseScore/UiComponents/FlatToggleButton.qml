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

FocusScope {
    id: root

    property alias navigation: navCtrl

    property alias icon: buttonIcon.iconCode
    property bool checked: false
    property alias backgroundColor: backgroundRect.color

    signal toggled

    implicitHeight: 30
    implicitWidth: 30

    opacity: root.enabled ? 1.0 : ui.theme.itemOpacityDisabled

    function ensureActiveFocus() {
        if (!root.activeFocus) {
            root.forceActiveFocus()
        }

        if (!navCtrl.active) {
            navCtrl.forceActive()
        }
    }

    onToggled: root.ensureActiveFocus()

    NavigationControl {
        id: navCtrl
        name: root.objectName != "" ? root.objectName : "FlatToggleButton"
        onTriggered: root.toggled()
    }

    Rectangle {
        id: backgroundRect

        anchors.fill: parent

        border.width: navCtrl.active ? 2 : 0
        border.color: ui.theme.focusColor

        color: ui.theme.buttonColor
        opacity: ui.theme.buttonOpacityNormal

        radius: 2
    }

    StyledIconLabel {
        id: buttonIcon

        anchors.fill: parent
    }

    MouseArea {
        id: clickableArea

        anchors.fill: parent

        hoverEnabled: true

        onReleased: {
            root.toggled()
        }
    }

    states: [
        State {
            name: "PRESSED"
            when: clickableArea.pressed

            PropertyChanges {
                target: backgroundRect
                color: ui.theme.accentColor
                opacity: ui.theme.accentOpacityHit
                border.width: 0
            }
        },

        State {
            name: "SELECTED"
            when: root.checked && !clickableArea.hovered

            PropertyChanges {
                target: backgroundRect
                color: ui.theme.accentColor
                opacity: ui.theme.accentOpacityNormal
                border.width: 0
            }
        },

        State {
            name: "HOVERED"
            when: clickableArea.containsMouse && !root.checked && !clickableArea.pressed

            PropertyChanges {
                target: backgroundRect
                color: ui.theme.buttonColor
                opacity: ui.theme.buttonOpacityHover
                border.color: ui.theme.strokeColor
                border.width: 1
            }
        },

        State {
            name: "SELECTED_HOVERED"
            when: clickableArea.containsMouse && root.checked

            PropertyChanges {
                target: backgroundRect
                color: ui.theme.accentColor
                opacity: ui.theme.accentOpacityHover
                border.width: 0
            }
        }
    ]
}
