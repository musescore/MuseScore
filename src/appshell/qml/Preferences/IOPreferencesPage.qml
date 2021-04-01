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

    Column {
        id: content

        width: parent.width
        spacing: 24

        readonly property int firstColumnWidth: 220

        MidiDevicesSection {
            currentInputDeviceIndex: ioModel.currentMidiInputDeviceIndex
            currentOutputDeviceIndex: ioModel.currentMidiOutputDeviceIndex
            inputDevices: ioModel.midiInputDeviceList()
            outputDevices: ioModel.midiOutputDeviceList()
            outputLatency: ioModel.midiOutputLatencyMilliseconds
            firstColumnWidth: content.firstColumnWidth

            onCurrentInputDeviceIndexChangeRequested: {
                ioModel.currentMidiInputDeviceIndex = newIndex
            }

            onCurrentOuputDeviceIndexChangeRequested: {
                ioModel.currentMidiOutputDeviceIndex = newIndex
            }

            onOutputLatencyChangeRequested: {
                ioModel.midiOutputLatencyMilliseconds = newLatencyMs
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
