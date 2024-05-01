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

import Muse.UiComponents 1.0
import MuseScore.Preferences 1.0

import "internal"

PreferencesPage {
    id: root

    AudioMidiPreferencesModel {
        id: audioMidiModel
    }

    Component.onCompleted: {
        audioMidiModel.init()
    }

    Column {
        width: parent.width
        spacing: root.sectionsSpacing

        AudioApiSection {
            currentAudioApiIndex: audioMidiModel.currentAudioApiIndex
            audioApiList: audioMidiModel.audioApiList()

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 1

            onCurrentAudioApiIndexChangeRequested: function(newIndex) {
                audioMidiModel.currentAudioApiIndex = newIndex
            }
        }

        SeparatorLine {}

        MidiDevicesSection {
            inputDeviceId: audioMidiModel.midiInputDeviceId
            outputDeviceId: audioMidiModel.midiOutputDeviceId
            inputDevices: audioMidiModel.midiInputDevices
            outputDevices: audioMidiModel.midiOutputDevices

            isMIDI20OutputSupported: audioMidiModel.isMIDI20OutputSupported
            useMIDI20Output: audioMidiModel.useMIDI20Output

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 2

            onInputDeviceIdChangeRequested: function(newId) {
                audioMidiModel.inputDeviceSelected(newId)
            }

            onOutputDeviceIdChangeRequested: function(newId) {
                audioMidiModel.outputDeviceSelected(newId)
            }

            onUseMIDI20OutputChangeRequested: function(use) {
                audioMidiModel.useMIDI20Output = use
            }
        }

        SeparatorLine {}

        JackSection {
            jackTransportEnable: playbackModel.jackTransportEnable

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 3

            onJackTransportEnableChangeRequested: function(enable) {
                playbackModel.jackTransportEnable = enable
            }
        }

        SeparatorLine {}

        MixerSection {
            muteHiddenInstruments: audioMidiModel.muteHiddenInstruments

            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 3

            onMuteHiddenInstrumentsChangeRequested: function(mute) {
                audioMidiModel.muteHiddenInstruments = mute
            }
        }

        /*
         * TODO: https://github.com/musescore/MuseScore/issues/9807
        SeparatorLine {}

        AudioEngineSection {
            navigation.section: root.navigationSection
            navigation.order: root.navigationOrderStart + 3

            onRestartAudioAndMidiDevicesRequested: {
                ioModel.restartAudioAndMidiDevices()
            }
        }
         */
    }
}
