import QtQuick 2.15

import MuseScore.UiComponents 1.0
import MuseScore.Preferences 1.0

Column {
    spacing: 18

    property var preferencesModel: null

    StyledTextLabel {
        text: qsTrc("appshell", "MIDI")
        font: ui.theme.bodyBoldFont
    }

    ComboBoxWithTitle {
        title: qsTrc("appshell", "Shortest note:")
        titleWidth: 220

        currentIndex: control.indexOfValue(preferencesModel.currentShortestNote)
        model: preferencesModel.shortestNotes()

        control.textRoleName: "title"
        control.valueRoleName: "value"

        onValueEdited: {
            preferencesModel.currentShortestNote = newValue
        }
    }
}
