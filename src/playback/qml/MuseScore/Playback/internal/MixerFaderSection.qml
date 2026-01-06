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

pragma ComponentBehavior: Bound

import QtQuick

import Muse.Ui
import Muse.UiComponents

MixerPanelSection {
    id: root

    Item {
        id: content

        required property MixerChannelItem channelItem

        height: childrenRect.height
        width: root.channelItemWidth

        property string accessibleName: (Boolean(root.needReadChannelName) ? channelItem.title + " " : "") + root.headerTitle

        Row {
            anchors.horizontalCenter: parent.horizontalCenter

            spacing: 8

            VolumeSlider {
                volumeLevel: content.channelItem.volumeLevel
                stepSize: 1.0

                navigation.panel: content.channelItem.panel
                navigation.row: root.navigationRowStart
                navigation.accessible.name: content.accessibleName + " " + readableVolumeLevel
                navigation.onActiveChanged: {
                    if (navigation.active) {
                        root.navigateControlIndexChanged({row: navigation.row, column: navigation.column})
                    }
                }

                onVolumeLevelMoved: function(level) {
                    content.channelItem.volumeLevel = Math.round(level * 10) / 10
                }

                onIncreaseRequested: {
                    content.channelItem.volumeLevel += stepSize
                }

                onDecreaseRequested: {
                    content.channelItem.volumeLevel -= stepSize
                }
            }

            Row {
                anchors.verticalCenter: parent.verticalCenter

                spacing: 2

                VolumePressureMeter {
                    id: leftPressure
                    currentVolumePressure: content.channelItem.leftChannelPressure
                }

                VolumePressureMeter {
                    id: rightPressure
                    currentVolumePressure: content.channelItem.rightChannelPressure
                    showRuler: true
                }
            }
        }
    }
}
