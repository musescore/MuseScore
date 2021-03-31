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

    Row {
        spacing: 12

        StyledTextLabel {
            width: 208
            anchors.verticalCenter: parent.verticalCenter
            text: qsTrc("appshell", "Shortest note:")
            horizontalAlignment: Text.AlignLeft
        }

        StyledComboBox {
            implicitWidth: 208

            textRoleName: "title"
            valueRoleName: "value"

            currentIndex: indexOfValue(preferencesModel.currentShortestNote)

            model: preferencesModel.shortestNotes()

            onValueChanged: {
                preferencesModel.currentShortestNote = value
            }
        }
    }
}
