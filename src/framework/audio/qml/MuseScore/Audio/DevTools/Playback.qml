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
import MuseScore.UiComponents 1.0
import MuseScore.Ui 1.0
import MuseScore.Audio 1.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.3

Rectangle {
    id: root

    color: ui.theme.backgroundPrimaryColor

    WaveFormModel {
        id: waveModel

        onCurrentSignalAmplitudeChanged: {
            waveView.requestPaint()
        }
    }

    Rectangle {
        id: backgroundRect

        anchors.fill: contentRow

        color: ui.theme.backgroundSecondaryColor
    }

    RowLayout {
        id: contentRow

        width: root.width

        spacing: 8

        Column {
            id: controlsColumn

            spacing: 8

            KnobControl {
                id: panningKnob

                anchors.horizontalCenter: controlsColumn.horizontalCenter
            }

            VolumePressureMeter {
                id: volumePressureMeter

                anchors.horizontalCenter: panningKnob.horizontalCenter

                currentVolumePressure: waveModel.currentVolumePressure
                minDisplayedVolumePressure: waveModel.minDisplayedDbfs
                maxDisplayedVolumePressure: waveModel.maxDisplayedDbfs
            }
        }

        WaveFormView {
            id: waveView

            Layout.fillWidth: true

            currentSignalAmplitude: waveModel.currentSignalAmplitude
        }
    }
}
