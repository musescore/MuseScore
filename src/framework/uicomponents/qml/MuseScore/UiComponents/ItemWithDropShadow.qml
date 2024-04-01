/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2024 MuseScore BVBA and others
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

Item {
    id: root

    default property alias contentData: content.data
    property alias shadow: theShadow

    implicitWidth: content.implicitWidth
    implicitHeight: content.implicitHeight

    StyledDropShadow {
        id: theShadow
        anchors.fill: contentContainer
        source: contentContainer
    }

    Item {
        id: contentContainer

        readonly property real theMargin: Math.abs(theShadow.horizontalOffset) + Math.abs(theShadow.verticalOffset) + Math.abs(theShadow.radius)
        anchors.fill: parent
        anchors.margins: -theMargin

        Item {
            id: content
            anchors.fill: parent
            anchors.margins: contentContainer.theMargin
        }
    }
}
