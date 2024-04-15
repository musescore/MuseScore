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

import Muse.Ui 1.0
import Muse.UiComponents 1.0
import Muse.Audio 1.0

MixerPanelSection {
    id: root

    Item {
        id: content

        height: childrenRect.height
        width: root.channelItemWidth

        property string accessibleName: (Boolean(root.needReadChannelName) ? channelItem.title + " " : "") + root.headerTitle

        Row {
            anchors.horizontalCenter: parent.horizontalCenter

            spacing: 8

            VolumeSlider {
                volumeLevel: channelItem.volumeLevel
                stepSize: 1.0

                navigation.panel: channelItem.panel
                navigation.row: root.navigationRowStart
                navigation.accessible.name: content.accessibleName + " " + readableVolumeLevel
                navigation.onActiveChanged: {
                    if (navigation.active) {
                        root.navigateControlIndexChanged({row: navigation.row, column: navigation.column})
                    }
                }

                onVolumeLevelMoved: function(level) {
                    channelItem.volumeLevel = Math.round(level * 10) / 10
                }

                onIncreaseRequested: {
                    channelItem.volumeLevel += stepSize
                }

                onDecreaseRequested: {
                    channelItem.volumeLevel -= stepSize
                }
            }

            Row {
                anchors.verticalCenter: parent.verticalCenter

                spacing: 2

                VolumePressureMeter {
                    id: leftPressure
                    currentVolumePressure: channelItem.leftChannelPressure
                }

                VolumePressureMeter {
                    id: rightPressure
                    currentVolumePressure: channelItem.rightChannelPressure
                    showRuler: true
                }
            }
        }
    }
}
