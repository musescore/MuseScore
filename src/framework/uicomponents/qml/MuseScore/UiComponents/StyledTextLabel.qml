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
import QtQuick 2.12

Text {
    id: root

    readonly property bool isEmpty: text.length === 0

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

    onLinkActivated: Qt.openUrlExternally(link)

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.NoButton
        cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.ArrowCursor
    }
}
