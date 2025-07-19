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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.GraphicalEffects 1.0

Item {
    id: root

    property string url: ""
    property int side: 0
    property bool withBackground: true

    width: side
    height: side

    Rectangle {
        id: background
        anchors.fill: parent

        color: ui.theme.backgroundSecondaryColor
        radius: root.side / 2
        border.width: 1
        border.color: ui.theme.strokeColor

        visible: Boolean(root.withBackground)
    }

    Loader {
        anchors.fill: parent
        anchors.margins: Boolean(root.withBackground) ? background.border.width : 0

        sourceComponent: Boolean(root.url) ? avatarComp : stubAvatarComp
    }

    Component {
        id: stubAvatarComp
        StyledIconLabel {
            anchors.centerIn: parent.centerIn
            iconCode: IconCode.ACCOUNT
            font.pixelSize: root.side / 2
        }
    }

    Component {
        id: avatarComp
        Image {
            source: root.url
            sourceSize: Qt.size(width, height)
            fillMode: Image.PreserveAspectCrop

            layer.enabled: ui.isEffectsAllowed
            layer.effect: EffectOpacityMask {
                maskSource: Rectangle {
                    width: root.side
                    height: root.side
                    radius: width / 2
                    visible: false
                }
            }
        }
    }
}


