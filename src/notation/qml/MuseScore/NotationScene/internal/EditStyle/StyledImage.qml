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
import QtQuick.Layouts 1.15

import MuseScore.NotationScene 1.0
import Muse.UiComponents 1.0
import Muse.Ui 1.0

Rectangle {
    id: root

    property double horizontalPadding: 8
    property double verticalPadding: 8

    property double forceWidth: 0
    property double forceHeight: 0
    property alias source: image.source

    width: image.width + 2 * horizontalPadding + border.width * 2
    height: image.height + 2 * verticalPadding + border.width * 2

    color: "#ffffff"
    border.color: ui.theme.strokeColor
    border.width: 1
    radius: 3

    Image {
        id: image
        width: forceWidth != 0 ? forceWidth : forceHeight != 0 ? sourceSize.width * (forceHeight / sourceSize.height) : sourceSize.width
        height: forceHeight != 0 ? forceHeight : forceWidth != 0 ? sourceSize.height * (forceWidth / sourceSize.width) : sourceSize.height
        anchors.centerIn: parent
        mipmap: true
        fillMode: Image.PreserveAspectFit
        opacity: enabled ? 1.0 : 0.2
    }
}


