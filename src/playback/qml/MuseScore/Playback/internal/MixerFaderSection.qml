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

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Audio 1.0

MixerPanelSection {
    id: root

    Item {
        id: content

        height: childrenRect.height
        width: root.delegateDefaultWidth

        property string accessibleName: (Boolean(root.needReadChannelName) ? item.title + " " : "") + root.headerTitle

        Component.onCompleted: {
            root.navigationRowEnd = root.navigationRowStart + 1 // todo
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter

            spacing: 8

            VolumeSlider {
                volumeLevel: item.volumeLevel
                stepSize: 1.0

                navigation.panel: item.panel
                navigation.row: root.navigationRowStart
                navigation.accessible.name: content.accessibleName + " " + readableVolumeLevel
                navigation.onActiveChanged: {
                    if (navigation.active) {
                        root.navigateControlNameChanged(navigation.name)
                    }
                }

                onVolumeLevelMoved: {
                    item.volumeLevel = Math.round(level * 10) / 10
                }

                onIncreaseRequested: {
                    item.volumeLevel += stepSize
                }

                onDecreaseRequested: {
                    item.volumeLevel -= stepSize
                }
            }

            Row {
                anchors.verticalCenter: parent.verticalCenter

                spacing: 2

                VolumePressureMeter {
                    id: leftPressure
                    currentVolumePressure: item.leftChannelPressure
                }

                VolumePressureMeter {
                    id: rightPressure
                    currentVolumePressure: item.rightChannelPressure
                    showRuler: true
                }
            }
        }
    }
}
