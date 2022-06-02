// Mirror intervals chromatically about a given pivot note.
// Copyright (C) 2018  Bill Hails
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

import MuseScore 3.0
import QtQuick 2.2
import QtQuick.Controls 1.1
import QtQuick.Controls.Styles 1.3
import QtQuick.Layouts 1.1
import QtQuick.Dialogs 1.1

MuseScore {
    version: "3.0.2"
    menuPath: "Plugins.Composing Tools.Mirror Intervals"
    description: "Mirrors (inverts) intervals about a given pivot note"
    pluginType: "dialog"
    width: 250
    height: 150

    onRun: {
        if (!curScore) {
            error("No score open.\nThis plugin requires an open score to run.\n")
            Qt.quit()
        }
    }

    function applyMirrorIntervals()
    {
        var selection = getSelection()
        if (selection === null) {
            error("No selection.\nThis plugin requires a current selection to run.\n")
            Qt.quit()
        }
        curScore.startCmd()
        mapOverSelection(selection, filterNotes, mirrorIntervals(getMirrorType(), getPivotNote()))
        curScore.endCmd()
    }

    function mapOverSelection(selection, filter, process) {
        selection.cursor.rewind(1)
        for (
            var segment = selection.cursor.segment;
                segment && segment.tick < selection.endTick;
                segment = segment.next
        ) {
            for (var track = selection.startTrack; track < selection.endTrack; track++) {
                var element = segment.elementAt(track)
                if (element) {
                    if (filter(element)) {
                        process(element, track)
                    }
                }
            }
        }
    }

    function filterNotes(element)
    {
        return element.type == Ms.CHORD
    }

    function mirrorIntervals(mirrorType, pivotNote)
    {
        if (mirrorType == 0) {
            return diatonicMirror(pivotNote)
        } else {
            return chromaticMirror(pivotNote)
        }
    }

    function chromaticMirror(pivotNote)
    {
        var pivots = [];
        return function(chord, track) {
            for (var i = 0; i < chord.notes.length; i++) {
                var note = chord.notes[i]
                note.tpc1 = lookupTpc(pivotNote, note.tpc1)
                note.tpc2 = lookupTpc(pivotNote, note.tpc2)
                if (!(track in pivots)) {
                    pivots[track] = nearestPivot(pivotNote, note.pitch);
                }
                note.pitch = performPivot(pivots[track], note.pitch);
            }
        }
    }

    function nearestPivot(pivotNote, pitch)
    {
        var root = pitch - (pitch % 12)
        var pivot =  root + pivotNote
        if ((pitch - pivot) > 6) {
            pivot += 12
        } else if ((pitch - pivot) < -6) {
            pivot -= 12
        }
        return pivot
    }

    function performPivot(pivot, pitch)
    {
        var diff = pivot - pitch;
        return pivot + diff
    }

    function diatonicMirror(pivotNote)
    {
        return function(chord) {
            error("diatonic\nnot implemented yet");
        }
    }

    function getSelection() {
        var cursor = curScore.newCursor()
        cursor.rewind(1)
        if (!cursor.segment) {
            return null
        }
        var selection = {
            cursor: cursor,
            startTick: cursor.tick,
            endTick: null,
            startStaff: cursor.staffIdx,
            endStaff: null,
            startTrack: null,
            endTrack: null
        }
        cursor.rewind(2)
        selection.endStaff = cursor.staffIdx + 1
        if (cursor.tick == 0) {
            selection.endTick = curScore.lastSegment.tick + 1
        } else {
            selection.endTick = cursor.tick
        }
        selection.startTrack = selection.startStaff * 4
        selection.endTrack = selection.endStaff * 4
        return selection
    }

    property int mirrorType: 1

    function getMirrorType()
    {
        return mirrorType
    }

    function getPivotNote()
    {
        return pivotNote.model.get(pivotNote.currentIndex).note
    }

    function error(errorMessage) {
        errorDialog.text = qsTr(errorMessage)
        errorDialog.open()
    }

    property var tpcMap: [
        [30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 09, 08, 07, 06, 05, 04, 03, 02, 01, 00, 23, 22, 21, 20],
        [32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 09, 08, 07, 06, 05, 04, 03, 02, 01, 00, 23, 22],
        [34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 09, 08, 07, 06, 05, 04, 03, 02, 01, 00],
        [12, 11, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 09, 08, 07, 06, 05, 04, 03, 02],
        [14, 13, 12, 11, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 09, 08, 07, 06, 05, 04],
        [28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 09, 08, 07, 06, 05, 04, 03, 02, 01, 00, 23, 22, 21, 20, 19, 18],
        [30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 09, 08, 07, 06, 05, 04, 03, 02, 01, 00, 23, 22, 21, 20],
        [32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 09, 08, 07, 06, 05, 04, 03, 02, 01, 00, 23, 22],
        [34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 09, 08, 07, 06, 05, 04, 03, 02, 01, 00],
        [12, 11, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 09, 08, 07, 06, 05, 04, 03, 02],
        [14, 13, 12, 11, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 09, 08, 07, 06, 05, 04],
        [28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 09, 08, 07, 06, 05, 04, 03, 02, 01, 00, 23, 22, 21, 20, 19, 18]
    ];

    function lookupTpc(pivot, tpc)
    {
        // tpc starts at -1
        return tpcMap[pivot][tpc + 1] - 1;
    }

    Rectangle {
        color: "lightgrey"
        anchors.fill: parent

        GridLayout {
            columns: 2
            anchors.fill: parent
            anchors.margins: 10
            Label {
                text: "Pivot"
            }
            ComboBox {
                id: pivotNote
                model: ListModel {
                    id: pivotNoteList
                    ListElement { text: "G";  note: 7;  }
                    ListElement { text: "G♯"; note: 8;  }
                    ListElement { text: "A";  note: 9;  }
                    ListElement { text: "B♭"; note: 10; }
                    ListElement { text: "B";  note: 11; }
                    ListElement { text: "C";  note: 0;  }
                    ListElement { text: "C♯"; note: 1;  }
                    ListElement { text: "D";  note: 2;  }
                    ListElement { text: "E♭"; note: 3;  }
                    ListElement { text: "E";  note: 4;  }
                    ListElement { text: "F";  note: 5;  }
                    ListElement { text: "F♯"; note: 6;  }
                }
                currentIndex: 5
                style: ComboBoxStyle {
                    font.family: 'MScore Text'
                    font.pointSize: 14
                }
            }
            Button {
                id: applyButton
                text: qsTranslate("PrefsDialogBase", "Apply")
                onClicked: {
                    applyMirrorIntervals()
                    Qt.quit()
                }
            }
            Button {
                id: cancelButton
                text: qsTranslate("PrefsDialogBase", "Cancel")
                onClicked: {
                    Qt.quit()
                }
            }
        }
    }

    MessageDialog {
        id: errorDialog
        title: "Error"
        text: ""
        onAccepted: {
            Qt.quit()
        }
        visible: false
    }
}
