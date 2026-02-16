/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

import MuseScore.AppShell

Item {
    id: root

    property var model: newInstanceLoadingScreenModel

    implicitWidth: model ? model.width : 288
    implicitHeight: model ? model.height : 80
    width: implicitWidth
    height: implicitHeight

    Rectangle {
        anchors.fill: parent
        color: model ? model.backgroundColor : "#FFFFFF"
    }

    Text {
        id: messageText
        anchors.fill: parent
        anchors.margins: 8

        text: model ? model.message : ""

        color: model ? model.messageColor : "#000000"
        font.family: model ? model.fontFamily : "FreeSans"
        font.pixelSize: model ? model.fontSize : 12
        font.bold: true

        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideMiddle
    }
}
