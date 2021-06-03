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
import MuseScore.Preferences 1.0

import "internal"

PreferencesPage {
    id: root

    contentHeight: content.height

    IOPreferencesModel {
        id: ioModel
    }

    Component.onCompleted: {
        ioModel.init()
    }

    Column {
        id: content

        width: parent.width
        spacing: 24

        readonly property int firstColumnWidth: 220

        AudioApiSection {
            currentAudioApiIndex: ioModel.currentAudioApiIndex
            audioApiList: ioModel.audioApiList()
            firstColumnWidth: content.firstColumnWidth

            onCurrentAudioApiIndexChangeRequested: {
                ioModel.currentAudioApiIndex = newIndex
            }
        }

        SeparatorLine {}

        MidiDevicesSection {
            currentInputDeviceIndex: ioModel.currentMidiInputDeviceIndex
            currentOutputDeviceIndex: ioModel.currentMidiOutputDeviceIndex
            inputDevices: ioModel.midiInputDevices
            outputDevices: ioModel.midiOutputDevices
            firstColumnWidth: content.firstColumnWidth

            onCurrentInputDeviceIndexChangeRequested: {
                ioModel.currentMidiInputDeviceIndex = newIndex
            }

            onCurrentOuputDeviceIndexChangeRequested: {
                ioModel.currentMidiOutputDeviceIndex = newIndex
            }
        }

        SeparatorLine {}

        AudioEngineSection {
            onRestartAudioAndMidiDevicesRequested: {
                ioModel.restartAudioAndMidiDevices()
            }
        }
    }
}
