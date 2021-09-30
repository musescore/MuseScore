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

    headerTitle: qsTrc("playback", "Volume")

    Item {
        height: contentColumn.implicitHeight
        width: root.delegateDefaultWidth

        Column {
            id: contentColumn

            anchors.horizontalCenter: parent.horizontalCenter

            width: parent.width

            spacing: 4

            TextInputField {
                id: volumeTextInputField

                anchors.horizontalCenter: parent.horizontalCenter

                height: 24
                width: 46

                textHorizontalAlignment: Qt.AlignHCenter

                validator: DoubleInputValidator {
                    id: doubleInputValidator
                    top: 12.0
                    bottom: -60.0
                    decimal: 1
                }

                currentText: Math.round(item.volumeLevel * 10) / 10

                onCurrentTextEdited: {
                    if (item.volumeLevel !== Number(newTextValue)) {
                        item.volumeLevel = Number(newTextValue)
                    }
                }
            }

            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width

                spacing: 8

                VolumeSlider {
                    volumeLevel: item.volumeLevel

                    onVolumeLevelMoved: {
                        item.volumeLevel = Math.round(level * 10) / 10
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

            Row {
                anchors.horizontalCenter: parent.horizontalCenter

                spacing: 6

                FlatToggleButton {
                    id: muteButton

                    height: 20
                    width: 20

                    icon: IconCode.MUTE
                    checked: item.muted
                    onToggled: {
                        item.muted = !item.muted
                    }
                }

                FlatToggleButton {
                    id: soloButton

                    height: 20
                    width: 20

                    icon: IconCode.SOLO
                    checked: item.solo
                    onToggled: {
                        item.solo = !item.solo
                    }
                }
            }
        }
    }
}
