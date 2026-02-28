/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
import QtQuick

pragma ComponentBehavior: Bound

Text {
    id: root

    readonly property bool isEmpty: text.length === 0
    property bool displayTruncatedTextOnHover: false

    color: ui.theme.fontPrimaryColor
    linkColor: ui.theme.linkColor
    opacity: root.enabled ? 1.0 : ui.theme.itemOpacityDisabled

    elide: Text.ElideRight
    verticalAlignment: Text.AlignVCenter
    horizontalAlignment: Text.AlignHCenter

    font {
        family: ui.theme.bodyFont.family
        pixelSize: ui.theme.bodyFont.pixelSize
    }

    onLinkActivated: function(link) {
        Qt.openUrlExternally(link)
    }

    Loader {
        id: mouseAreaLoader
        anchors.fill: parent
        active: (root.displayTruncatedTextOnHover && root.truncated) || root.hoveredLink

        sourceComponent: MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.NoButton
            cursorShape: root.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
            hoverEnabled: true

            onPressed: {
                ui.tooltip.hide(root, true)
            }

            onContainsMouseChanged: {
                if (!containsMouse || !root.truncated || root.hoveredLink) {
                    ui.tooltip.hide(root)
                    return
                }
                ui.tooltip.show(root, root.text)
            }
        }
    }
}
