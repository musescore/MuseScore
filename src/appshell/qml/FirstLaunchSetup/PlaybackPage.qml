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
import QtQuick.Window 2.15

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.GraphicalEffects 1.0
import MuseScore.AppShell 1.0

Page {
    title: qsTrc("appshell/gettingstarted", "Playback")
    explanation: qsTrc("appshell/gettingstarted", "Discover new sound libraries in MuseHub and enjoy realistic score playback")

    titleContentSpacing: 12

    extraButtonTitle: qsTrc("appshell/gettingstarted", "Watch video")

    onExtraButtonClicked: {
        Qt.openUrlExternally("https://youtu.be/L8OdWHOjL8c")
    }

    Image {
        id: image
        anchors.fill: parent
        fillMode: Image.PreserveAspectCrop
        source: "resources/MuseSounds.png"
        sourceSize: Qt.size(width * Screen.devicePixelRatio, height * Screen.devicePixelRatio)

        layer.enabled: ui.isEffectsAllowed
        layer.effect: EffectOpacityMask {
            maskSource: Rectangle {
                width: image.width
                height: image.height
                radius: 3
            }
        }
    }
}
