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

    property var model: loadingScreenModel

    implicitWidth: 810
    implicitHeight: 406
    width: implicitWidth
    height: implicitHeight

    Image {
        id: background

        anchors.fill: parent

        source: "../resources/splash_screen.svg"
        fillMode: Image.Stretch
    }

    Text {
        id: messageText
        x: 48
        y: 230

        text: model ? model.message : ""

        color: "#F1F1EE"
        font.family: model ? model.fontFamily : "FreeSans"
        font.pixelSize: model ? model.fontSize : 12
    }

    Text {
        id: versionText

        anchors.right: parent.right
        anchors.rightMargin: 48
        anchors.bottom: websiteText.top
        anchors.bottomMargin: 5

        text: model ? model.versionString : ""

        color: "#19F3FF"
        font.family: model ? model.fontFamily : "FreeSans"
        font.pixelSize: model ? model.fontSize : 12
        horizontalAlignment: model && model.isRtl ? Text.AlignLeft : Text.AlignRight
    }

    Text {
        id: websiteText

        anchors.right: parent.right
        anchors.rightMargin: 48
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 48

        text: model ? model.website : ""

        color: "#F1F1EE"
        font.family: model ? model.fontFamily : "FreeSans"
        font.pixelSize: model ? model.fontSize : 12
        horizontalAlignment: model && model.isRtl ? Text.AlignLeft : Text.AlignRight
    }
}
