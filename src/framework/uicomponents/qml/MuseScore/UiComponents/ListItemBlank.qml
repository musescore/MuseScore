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

FocusableControl {
    id: root

    property string hint

    property bool isSelected: false
    property alias radius: root.background.radius

    property color normalColor: "transparent"
    property color hoverHitColor: ui.theme.buttonColor
    property color selectedColor: ui.theme.accentColor

    signal clicked(var mouse)
    signal doubleClicked(var mouse)
    signal hovered(var isHovered, var mouseX, int mouseY)

    implicitHeight: 30
    implicitWidth: Boolean(ListView.view) ? ListView.view.width : 30

    Accessible.selectable: true
    Accessible.selected: isSelected

    navigation.accessible.role: MUAccessible.ListItem

    background.color: normalColor
    background.opacity: root.enabled ? 1 : ui.theme.itemOpacityDisabled

    focusBorder.drawOutsideParent: false

    mouseArea.hoverEnabled: root.visible
    mouseArea.onHoveredChanged: root.hovered(mouseArea.containsMouse, mouseArea.mouseX, mouseArea.mouseY)

    mouseArea.onClicked: function(mouse) { root.clicked(mouse) }
    mouseArea.onDoubleClicked: function(mouse) { root.doubleClicked(mouse) }

    mouseArea.onContainsMouseChanged: {
        if (!Boolean(root.hint)) {
            return
        }

        if (mouseArea.containsMouse) {
            ui.tooltip.show(this, root.hint)
        } else {
            ui.tooltip.hide(this)
        }
    }

    onNavigationTriggered: root.clicked(null)

    states: [
        State {
            name: "HOVERED"
            when: mouseArea.containsMouse && !mouseArea.pressed && !root.isSelected

            PropertyChanges {
                target: root.background
                opacity: ui.theme.buttonOpacityHover
                color: root.hoverHitColor
            }
        },

        State {
            name: "PRESSED"
            when: mouseArea.pressed && !root.isSelected

            PropertyChanges {
                target: root.background
                opacity: ui.theme.buttonOpacityHit
                color: root.hoverHitColor
            }
        },

        State {
            name: "SELECTED"
            when: !mouseArea.containsMouse && !mouseArea.pressed && root.isSelected

            PropertyChanges {
                target: root.background
                opacity: ui.theme.accentOpacityNormal
                color: ui.theme.accentColor
            }
        },

        State {
            name: "SELECTED_HOVERED"
            when: mouseArea.containsMouse && !mouseArea.pressed && root.isSelected

            PropertyChanges {
                target: root.background
                opacity: ui.theme.accentOpacityHover
                color: ui.theme.accentColor
            }
        },

        State {
            name: "SELECTED_PRESSED"
            when: mouseArea.pressed && root.isSelected

            PropertyChanges {
                target: root.background
                opacity: ui.theme.accentOpacityHit
                color: ui.theme.accentColor
            }
        }
    ]
}
