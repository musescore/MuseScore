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
import MuseScore.Playback 1.0

Item {
    id: root

    Rectangle {
        id: backgroundRect

        anchors.fill: parent
        color: ui.theme.backgroundPrimaryColor
    }

    ListView {
        anchors.fill: parent

        orientation: Qt.Horizontal

        spacing: 4

        model: MixerPanelModel {
            id: mixerPanelModel

            Component.onCompleted: {
                mixerPanelModel.load()
            }
        }

        delegate: Column {

            spacing: 2

            KnobControl {
                id: balanceKnob

                value: item.balance * balanceKnob.valueScale

                onMoved: {
                    item.balance = value / balanceKnob.valueScale
                }
            }

            Row {

                spacing: 8

                VolumeSlider {

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

            Rectangle {
                width: parent.width
                height: 22

                color: Utils.colorWithAlpha(ui.theme.accentColor, 0.5)
                border.color: ui.theme.accentColor
                border.width: 1

                StyledTextLabel {
                    anchors.centerIn: parent
                    text: item.title
                }
            }
        }
    }
}
