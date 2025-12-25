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

Rectangle {
    id: root

    property real horizontalPadding: 8
    property real verticalPadding: 8

    property real forceWidth: 0
    property real forceHeight: 0
    property alias source: image.source

    width: image.width + 2 * horizontalPadding + border.width * 2
    height: image.height + 2 * verticalPadding + border.width * 2

    color: "#ffffff"
    border.color: ui.theme.strokeColor
    border.width: 1

    Image {
        id: image
        width: root.forceWidth != 0 ? root.forceWidth 
               : root.forceHeight != 0 ? sourceSize.width * (root.forceHeight / sourceSize.height) 
               : sourceSize.width
        height: root.forceHeight != 0 ? root.forceHeight 
                : root.forceWidth != 0 ? sourceSize.height * (root.forceWidth / sourceSize.width) 
                : sourceSize.height
        anchors.centerIn: parent
        mipmap: true
        fillMode: Image.PreserveAspectFit
        opacity: enabled ? 1.0 : 0.2
    }
}
