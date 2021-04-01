import QtQuick 2.15
import QtQuick.Layouts 1.12

import MuseScore.UiComponents 1.0

Column {
    id: root

    signal restartAudioAndMidiDevicesRequested()

    spacing: 18

    StyledTextLabel {
        text: qsTrc("appshell", "Audio Engine")
        font: ui.theme.bodyBoldFont
    }

    FlatButton {
        text: qsTrc("appshell", "Restart audio and MIDI Devices")

        onClicked: {
            root.restartAudioAndMidiDevicesRequested()
        }
    }
}
