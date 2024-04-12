/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

import Muse.UiComponents 1.0

Column {
    id: root

    property alias icon: iconLabel.iconCode
    property alias text: textLabel.text

    anchors.horizontalCenter: parent.horizontalCenter
    anchors.verticalCenter: parent.verticalCenter

    spacing: 10

    StyledIconLabel {
        id: iconLabel

        anchors.horizontalCenter: parent.horizontalCenter
        height: 50

        font.pixelSize: 64
        renderType: Text.NativeRendering
    }

    StyledTextLabel {
        id: textLabel

        anchors.horizontalCenter: parent.horizontalCenter
    }
}
