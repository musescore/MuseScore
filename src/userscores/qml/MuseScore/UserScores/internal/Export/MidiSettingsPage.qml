import QtQuick 2
import QtQuick.Layouts 1

import MuseScore.UiComponents 1
import MuseScore.UserScores 1

ColumnLayout {
    id: root
    spacing: 12

    property ExportDialogModel model
    property int firstColumnWidth

    CheckBox {
        text: qsTrc("userscores", "Expand repeats")
        checked: root.model.midiExpandRepeats
        onClicked: {
            root.model.midiExpandRepeats = !checked
        }
    }

    CheckBox {
        text: qsTrc("userscores", "Export RPNs")
        checked: root.model.midiExportRpns
        onClicked: {
            root.model.midiExportRpns = !checked
        }
    }

    StyledTextLabel {
        Layout.fillWidth: true
        text: qsTrc("userscores", "Each selected part will be exported as a separate MIDI file.")
        horizontalAlignment: Text.AlignLeft
        wrapMode: Text.WordWrap
    }
}
