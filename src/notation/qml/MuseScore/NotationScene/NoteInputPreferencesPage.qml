import QtQuick 2.15
import QtQuick.Layouts 1.12

import MuseScore.Ui 1.0
import MuseScore.UiComponents 1.0
import MuseScore.Preferences 1.0

Item {
    NoteInputPreferencesModel {
        id: noteInputModel
    }

    Column {
        anchors.fill: parent

        spacing: 22

        StyledTextLabel {
            text: qsTrc("notation", "Note Input")
            font: ui.theme.bodyBoldFont
        }

        CheckBox {
            text: qsTrc("notation", "Advance to next note on key release (MIDI)")

            checked: noteInputModel.advanceToNextNoteOnKeyRelease

            onClicked: {
                noteInputModel.advanceToNextNoteOnKeyRelease = !checked
            }
        }

        CheckBox {
            width: 170
            text: qsTrc("notation", "Colour notes outside of usable pitch range")

            checked: noteInputModel.colorNotesOusideOfUsablePitchRange

            onClicked: {
                noteInputModel.colorNotesOusideOfUsablePitchRange = !checked
            }
        }

        Row {
            width: parent.width
            height: childrenRect.height

            spacing: 46

            StyledTextLabel {
                width: 173

                horizontalAlignment: Qt.AlignLeft
                wrapMode: Text.WordWrap
                maximumLineCount: 2

                text: qsTrc("notation", "Delay between notes in automatic real time mode:")
            }

            IncrementalPropertyControl {
                width: 102

                currentValue: noteInputModel.delayBetweenNotesInRealTimeModeMilliseconds
                measureUnitsSymbol: qsTrc("notation", "ms")

                onValueEdited: {
                    noteInputModel.delayBetweenNotesInRealTimeModeMilliseconds = newValue
                }
            }
        }

        SeparatorLine {}

        CheckBox {
            text: qsTrc("notation", "Play Notes When Editing")
            textFont: ui.theme.bodyBoldFont

            checked: noteInputModel.playNotesWhenEditing

            onClicked: {
                noteInputModel.playNotesWhenEditing = !checked
            }
        }

        Row {
            enabled: noteInputModel.playNotesWhenEditing
            spacing: 126

            StyledTextLabel {
                anchors.verticalCenter: parent.verticalCenter

                width: 93
                horizontalAlignment: Qt.AlignLeft

                text: qsTrc("notation", "Default duration:")
            }

            IncrementalPropertyControl {
                width: 102

                currentValue: noteInputModel.notePlayDurationMilliseconds
                measureUnitsSymbol: qsTrc("notation", "ms")

                onValueEdited: {
                    noteInputModel.notePlayDurationMilliseconds = newValue
                }
            }
        }

        CheckBox {
            text: qsTrc("notation", "Play chord when editing")

            checked: noteInputModel.playChordWhenEditing
            enabled: noteInputModel.playNotesWhenEditing

            onClicked: {
                noteInputModel.playChordWhenEditing = !checked
            }
        }

        CheckBox {
            text: qsTrc("notation", "Play chord symbol when editing")

            checked: noteInputModel.playChordSymbolWhenEditing
            enabled: noteInputModel.playNotesWhenEditing

            onClicked: {
                noteInputModel.playChordSymbolWhenEditing = !checked
            }
        }
    }
}
