//==============================================
//  courtesy accidentals v1.0
//
//  Copyright (C)2012-2019 Jörn Eichler (heuchi)
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
//==============================================

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import MuseScore 3.0
import Muse.UiComponents

MuseScore {

    width: 344
    height: 360

    // configuration
    property bool useBracket: false

    property var typeNextMeasure:  1
    property var typeNumMeasures:  2
    property var typeEvent:        3
    property var typeDodecaphonic: 4

    property var eventFullRest:      1
    property var eventDoubleBar:     2
    property var eventRehearsalMark: 4
    property var eventEndScore:      8 // we don't really need this, but...

    property var operationMode;
    property var numMeasures;
    property var eventTypes;

    Component.onCompleted: {
        console.log("MuseScore Version = "+mscoreVersion);
        console.log("MajorVersion = "+mscoreMajorVersion);
        console.log("MinorVersion = "+mscoreMinorVersion);
        console.log("UpdateVersion= "+mscoreUpdateVersion);

        // These options don't work in MuseScore v3
        optDoubleBar.checked = false;
        optDoubleBar.enabled = false;
        optDoubleBar.opacity = 0.5;
        optFullRest.checked = false;
        optFullRest.enabled = false;
        optFullRest.opacity = 0.5;
        optRehearsalMark.checked = false;
        optRehearsalMark.enabled = false;
        optRehearsalMark.opacity = 0.5;
    }

    // Error dialog

    MessageDialog {
        id: errorDialog
        visible: false
        //icon: StandardIcon.Warning
    }

    // Dialog window

    function setUseBracketState() {
        if (optDodecaphonic.checked == true) {
            // disable brackets
            optUseBracket.enabled = false;
            optUseBracket.opacity = 0.5;
        } else {
            optUseBracket.enabled = true;
            optUseBracket.opacity = 1.0;
        }
    }

    Item {
        id: rect1
        anchors.fill: parent
        anchors.margins: 8

        ColumnLayout {
            id: col1
            anchors.left: parent.left
            anchors.right: parent.right

            ButtonGroup {id: typeGroup}

            Label {
                text: "Add courtesy accidentals for"
            }

            RowLayout {
                Rectangle { // for indentation
                    width: 10
                }

                ColumnLayout {

                    Rectangle {height: 2}
                    RadioButton {
                        id: optNextMeasure
                        text: "notes up to the next measure"
                        checked: true
                        ButtonGroup.group: typeGroup
                        onClicked: { setUseBracketState(); }
                    }

                    Rectangle {height: 2}
                    RowLayout {
                        RadioButton {
                            id: optNumMeasures
                            text: "notes up to the next"
                            ButtonGroup.group: typeGroup
                            onClicked: { setUseBracketState(); }
                        }

                        SpinBox {
                            id: valNumMeasures
                            implicitWidth: 45
                            from: 2
                            to: 99
                        }

                        Label {
                            text: "measures"
                        }
                    }

                    RowLayout {
                        RadioButton {
                            Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                            id: optEvent
                            text: "notes up to the"
                            ButtonGroup.group: typeGroup
                            onClicked: { setUseBracketState(); }
                        }

                        ColumnLayout {
                            CheckBox {
                                id: optFullRest
                                text: "next full measure rest"
                                checked: true
                                onClicked: checked = !checked
                            }
                            CheckBox {
                                id: optDoubleBar
                                text: "next double bar line"
                                checked: true
                                onClicked: checked = !checked
                            }
                            CheckBox {
                                id: optRehearsalMark
                                text: "next rehearsal mark"
                                checked: false
                                onClicked: checked = !checked
                            }
                            CheckBox {
                                id: optEndScore
                                text: "end of the score"
                                checked: true
                                onClicked: checked = !checked
                            }
                        }
                    }

                    Rectangle {height: 2}
                    RadioButton {
                        id: optDodecaphonic
                        text:"all notes (dodecaphonic style)"
                        ButtonGroup.group: typeGroup
                        onClicked: { setUseBracketState(); }
                    }
                }
            }

            Rectangle {height: 4}

            // Parenthesis option
            CheckBox {
                id: optUseBracket
                text: "Put accidentals in parenthesis"
                checked: false
                onClicked: checked = !checked
            }

            // preserve user settings
            Settings {
                category: "CourtesyAccidentalPlugin"
                property alias typeNextMeasure: optNextMeasure.checked
                property alias typeNumMeasures: optNumMeasures.checked
                property alias valueNumMeasure: valNumMeasures.value
                property alias typeEvent: optEvent.checked
                property alias typeFullRest: optFullRest.checked
                property alias typeDoubleBar: optDoubleBar.checked
                property alias typeRehearsalM: optRehearsalMark.checked
                property alias typeEndScore: optEndScore.checked
                property alias valueDodecaphonic: optDodecaphonic.checked
                property alias valueUseBracket: optUseBracket.checked
            }
        }
        // The buttons

        FlatButton {
            text:"Add accidentals"
            anchors {
                top: col1.bottom
                topMargin: 15
                left: rect1.left
                leftMargin: 10
            }
            onClicked: {
                var hasError = false;

                // set configuration
                useBracket = optUseBracket.checked;

                // set type
                if (optNextMeasure.checked) {
                    operationMode = typeNextMeasure;
                } else if (optNumMeasures.checked) {
                    operationMode = typeNumMeasures;
                    numMeasures = valNumMeasures.value;
                } else if (optEvent.checked) {
                    operationMode = typeEvent;
                    eventTypes = 0;
                    if (optFullRest.checked) {
                        eventTypes |= eventFullRest;
                    }
                    if (optDoubleBar.checked) {
                        eventTypes |= eventDoubleBar;
                    }
                    if (optRehearsalMark.checked) {
                        eventTypes |= eventRehearsalMark;
                    }
                    if (optEndScore.checked) {
                        eventTypes |= eventEndScore;
                    }
                    if (!eventTypes) {
                        // show error: at least one item needs to be selected
                        //console.log("ERROR: configuration");
                        hasError = true;
                        errorDialog.text = "No terminating event selected";
                        errorDialog.visible = true;
                    }
                } else if (optDodecaphonic.checked) {
                    operationMode = typeDodecaphonic;
                }

                if (!hasError) {
                    curScore.startCmd();
                    addAcc();
                    curScore.endCmd();
                }
                quit();
            }
        }

        FlatButton {
            text: "Cancel"
            anchors {
                top: col1.bottom
                topMargin: 15
                right: rect1.right
                rightMargin: 10
            }
            onClicked: {
                quit();
            }
        }
    }

    // if nothing is selected process whole score
    property bool processAll: false

    // function tpcName
    //
    // return name of note

    function tpcName(tpc) {
        var tpcNames = new Array(
                    "Fbb", "Cbb", "Gbb", "Dbb", "Abb", "Ebb", "Bbb",
                    "Fb",   "Cb",   "Gb",   "Db",   "Ab",   "Eb",   "Bb",
                    "F",     "C",     "G",     "D",     "A",     "E",     "B",
                    "F#",   "C#",   "G#",   "D#",   "A#",    "E#",   "B#",
                    "F##", "C##", "G##", "D##", "A##",  "E##",  "B##"
                    );

        return(tpcNames[tpc+1]);
    }

    // function getEndStaffOfPart
    //
    // return the first staff that does not belong to
    // the part containing given start staff.

    function getEndStaffOfPart(startStaff) {
        var startTrack = startStaff * 4;
        var parts = curScore.parts;

        for(var i = 0; i < parts.length; i++) {
            var part = parts[i];

            if( (part.startTrack <= startTrack)
                    && (part.endTrack > startTrack) ) {
                return(part.endTrack/4);
            }
        }

        // not found!
        console.log("error: part for " + startStaff + " not found!");
        quit();
    }

    // function addAccidental
    //
    // add correct accidental to note

    function addAccidental(note) {
        if(note.accidental == null) {
            // calculate type of needed accidental
            var accidental=Accidental.NONE;
            if(note.tpc < 6) {
                accidental = Accidental.FLAT2;
            } else if(note.tpc < 13) {
                accidental = Accidental.FLAT;
            } else if(note.tpc < 20) {
                accidental = Accidental.NATURAL;
            } else if(note.tpc < 27) {
                accidental = Accidental.SHARP;
            } else {
                accidental = Accidental.SHARP2;
            }
            note.accidentalType = accidental;
            // put bracket on accidental if not in dodecaphonic mode
            if (operationMode != typeDodecaphonic
                    && note.accidental) {
                if(useBracket) {
                    note.accidental.accidentalBracket = 1;
                } else {
                    note.accidental.accidentalBracket = 0;
                }
            }
        }
    }

    // function processNote
    //
    // for each measure we create a table that contains
    // the actual 'noteName' of each 'noteClass'
    //
    // a 'noteClass' is the natural name of a space
    // or line of the staff and the octave:
    // C5, F6, B3 are 'noteClass'
    //
    // a 'noteName' would be C, F#, Bb for example
    // (we don't need the octave here)
    //
    // we also remember the measure number that note was found
    // if we operate in typeNumMeasures mode. Thus:
    //
    // curMeasureArray[<noteClass>] = [<noteName>,<measureNum>]

    function processNote(note,prevMeasureArray,curMeasureArray,curMeasureNum) {
        var octave=Math.floor(note.pitch/12);

        // use tpc1 instead of tpc for octave correction
        // since this will also work for transposing instruments
        // correct octave for Cb and Cbb
        if(note.tpc1 == 7 || note.tpc1 == 0) {
            octave++; // belongs to higher octave
        }
        // correct octave for B# and B##
        if(note.tpc1 == 26 || note.tpc1 == 33) {
            octave--; // belongs to lower octave
        }

        var noteName = tpcName(note.tpc);
        var noteClass = noteName.charAt(0)+octave;

        // remember note for next measure
        curMeasureArray[noteClass]=[noteName,curMeasureNum];

        if (operationMode == typeDodecaphonic) {
            addAccidental(note);
        } else if (typeof prevMeasureArray[noteClass] !== 'undefined') {
            // check if current note needs courtesy acc
            if(prevMeasureArray[noteClass][0] != noteName) {
                // this note needs an accidental
                // if there's none present anyway
                addAccidental(note);
            }
            // delete entry to make sure we don't create the
            // same accidental again in the same measure
            delete prevMeasureArray[noteClass];
        }
    }

    // function processPart
    //
    // do the actual work: process all given tracks in parallel
    // add courtesy accidentals where needed.
    //
    // We go through all tracks simultaneously, because we also want courtesy
    // accidentals for notes across different staves when they are in the
    // same octave and for notes of different voices in the same octave

    function processPart(cursor,endTick,startTrack,endTrack) {
        if(processAll) {
            // we need to reset track first, otherwise
            // rewind(0) doesn't work correctly
            cursor.track=0;
            cursor.rewind(0);
        } else {
            cursor.rewind(1);
        }

        var curMeasureNum = 0;
        var segment = cursor.segment;

        var curMeasureArray = new Array();
        var prevMeasureArray = new Array();

        // we use a segment, because the cursor always proceeds to
        // the next element in the given track and we don't know
        // in which track the next element is.

        while(segment && (processAll || segment.tick < endTick)) {
            // we search for key signatures and bar lines
            // in first voice of first staff:
            var keySigTrack = startTrack - (startTrack % 4);

            // check for new measure
            if(segment.elementAt(keySigTrack)
                    && segment.elementAt(keySigTrack).type == Element.BAR_LINE) {
                // if double bar line and in nextEvent mode check
                // if this leads to reset of prevMeasureArray

                curMeasureNum++;

                // depending on operationMode: update prevMeasureArray
                switch (operationMode) {
                case typeNextMeasure:
                    prevMeasureArray = curMeasureArray;
                    break;

                case typeNumMeasures:
                    // delete all entries that are too old
                    var toDelete = [];
                    for (var n in prevMeasureArray) {
                        if (curMeasureNum - prevMeasureArray[n][1] > numMeasures) {
                            toDelete.push(n);
                        }
                    }
                    // now delete, otherwise iterating (n in prevMeasureArray) will not work
                    for (var x = 0; x < toDelete.length; x++)
                        delete prevMeasureArray[toDelete[x]];
                    // fall through!
                case typeEvent:
                    // copy entries from curMeasureArray
                    for (var n in curMeasureArray) {
                        prevMeasureArray[n] = curMeasureArray[n];
                    }
                    break;
                }

                // if barline is double, might need to forget
                // previous mesaure...
                var barLine = segment.elementAt(keySigTrack);
                if ((operationMode==typeEvent)
                        && (eventTypes & eventDoubleBar)
                        && (barLine.barLineType == BarLine.DOUBLE)) {
                    prevMeasureArray = new Array();
                }

                // reset curMeasureArray
                curMeasureArray = new Array();
            }

            // check for new key signature
            // we only do this for the first track of the first staff
            // this means we miss the event of having two different
            // key signatures in different staves of the same part
            // This remains for future version if needed
            // we look inside this loop to make sure we don't miss
            // any segments. This could be improved for speed.
            // A KeySig that has generated == true was created by
            // layout, and is probably at the beginning of a new line
            // so we don't need it.

            if (segment.elementAt(keySigTrack)
                    && segment.elementAt(keySigTrack).type == Element.KEYSIG
                    && (!segment.elementAt(keySigTrack).generated)) {
                //console.log("found KEYSIG");
                // just forget the previous measure info
                // to not generate any courtesy accidentals
                prevMeasureArray = new Array();
            }

            // BUG: access to annotations is broken in 2.0.3
            //
            // check for rehearsal mark
            //var annotations = segment.annotations;

            //if (annotations && annotations.length > 0) {
            //      for (var i = 0; i < annotations.length; i++) {
            //            var mark = annotations[i];
            //            if (mark.type == Element.REHEARSAL_MARK) {
            //                  if (operationMode == typeEvent
            //                    && (eventTypes & eventRehearsalMark)) {
            //                        // reset array
            //                        prevMeasureArray = new Array();
            //                  }
            //                  console.log("found rehearsal mark");
            //            }
            //      }
            //}

            // if we find a full measure rest, it needs to be in the whole part
            var allTracksFullMeasureRest = true;
            var restFound = false;

            // scann music
            for(var track=startTrack; track<endTrack; track++) {
                // check for full measure rest
                if (segment.elementAt(track) && segment.elementAt(track).type == Element.REST) {
                    var rest = segment.elementAt(track);
                    if (!rest.isFullMeasure) {
                        allTracksFullMeasureRest = false;
                    } else {
                        restFound = true;
                    }
                }

                // look for notes and grace notes
                if (segment.elementAt(track) && segment.elementAt(track).type == Element.CHORD) {
                    // not a rest so unset allPartsFullMeasureRest
                    allTracksFullMeasureRest = false;
                    // process graceNotes if present
                    if(segment.elementAt(track).graceNotes.length > 0) {
                        var graceChords = segment.elementAt(track).graceNotes;

                        for(var j=0;j<graceChords.length;j++) {
                            var notes = graceChords[j].notes;
                            for(var i=0;i<notes.length;i++) {
                                processNote(notes[i],prevMeasureArray,curMeasureArray,curMeasureNum);
                            }
                        }
                    }

                    // process notes
                    var notes = segment.elementAt(track).notes;

                    for(var i=0;i<notes.length;i++) {
                        processNote(notes[i],prevMeasureArray,curMeasureArray,curMeasureNum);
                    }
                }
            }

            if (restFound && allTracksFullMeasureRest) {
                // reset prevMeasureArray if configured to do so
                if (operationMode == typeEvent
                        && (eventTypes & eventFullRest)) {
                    prevMeasureArray = new Array();
                }
            }
            segment=segment.next;
        }
    }

    function addAcc() {
        console.log("start add courtesy accidentals");

        if (typeof curScore === 'undefined' || curScore == null) {
            console.log("error: no score!");
            quit();
        }

        // find selection
        var startStaff;
        var endStaff;
        var endTick;

        var cursor = curScore.newCursor();
        cursor.rewind(1);
        if(!cursor.segment) {
            // no selection
            console.log("no selection: processing whole score");
            processAll = true;
            startStaff = 0;
            endStaff = curScore.nstaves;
        } else {
            startStaff = cursor.staffIdx;
            cursor.rewind(2);
            endStaff = cursor.staffIdx+1;
            endTick = cursor.tick;
            if(endTick == 0) {
                // selection includes end of score
                // calculate tick from last score segment
                endTick = curScore.lastSegment.tick + 1;
            }
            cursor.rewind(1);
            console.log("Selection is: Staves("+startStaff+"-"+endStaff+") Ticks("+cursor.tick+"-"+endTick+")");
        }

        console.log("ProcessAll is "+processAll);

        // go through all staves of a part simultaneously
        // find staves that belong to the same part

        var curStartStaff = startStaff;

        while(curStartStaff < endStaff) {
            // find end staff for this part
            var curEndStaff = getEndStaffOfPart(curStartStaff);

            if(curEndStaff > endStaff) {
                curEndStaff = endStaff;
            }

            // do the work
            processPart(cursor,endTick,curStartStaff*4,curEndStaff*4);

            // next part
            curStartStaff = curEndStaff;
        }
    }
}
