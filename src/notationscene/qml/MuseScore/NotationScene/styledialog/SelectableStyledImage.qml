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

import Muse.Ui
import Muse.UiComponents
import MuseScore.NotationScene

Column {
    id: root
    spacing: 8

    property alias source: image.source
    property alias forceWidth: image.forceWidth
    property alias forceHeight: image.forceHeight
    property alias horizontalPadding: image.horizontalPadding
    property alias verticalPadding: image.verticalPadding
    property alias text: label.text

    required property bool checked
    signal clicked()

    StyledImage {
        id: image

        border.width: root.checked ? 2 : 1
        border.color: root.checked ? ui.theme.accentColor : ui.theme.strokeColor

        MouseArea {
            id: mouseArea

            anchors.fill: parent
            onClicked: root.clicked()
        }
    }

    StyledTextLabel {
        id: label
        width: image.width
        horizontalAlignment: Qt.AlignHCenter
        wrapMode: Text.WordWrap
    }
}
