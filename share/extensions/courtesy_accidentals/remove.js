//==============================================
//  remove courtesy accidentals v1.0
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

function main() {
    curScore.startCmd()
    removeAcc()
    curScore.endCmd()

    quit()
}

// if nothing is selected process whole score
var processAll = false

// function tpcName
//
// return name of note
var TPC_NAMES = [
            "Fbb", "Cbb", "Gbb", "Dbb", "Abb", "Ebb", "Bbb",
            "Fb",   "Cb",   "Gb",   "Db",   "Ab",   "Eb",   "Bb",
            "F",     "C",     "G",     "D",     "A",     "E",     "B",
            "F#",   "C#",   "G#",   "D#",   "A#",    "E#",   "B#",
            "F##", "C##", "G##", "D##", "A##",  "E##",  "B##"
        ];

function tpcName(tpc) {

    return(TPC_NAMES[tpc+1]);
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
// curMeasureArray[<noteClass>] = <noteName>

function processNote(note,curMeasureArray,keySig) {
    var octave=Math.floor(note.pitch/12);

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

    // a tied back note never needs an accidental
    if (note.tieBack != null) {
        if(note.accidental != null) {
            // security checks
            var thisPitch = note.pitch;
            var thisAcc = note.accidentalType;

            // remove
            note.accidentalType = Accidental.NONE;

            // if pitch changed, we were wrong...
            if(note.pitch != thisPitch) {
                console.log("ERROR1: pitch of note changed!");
                //note.color = "#ff0000";
                note.accidentalType = thisAcc;
            }
        }
        // if the tied back note is not part of
        // the current key sig, we need to remember it.
        //if( ! (note.tpc > keySig+12 && note.tpc < keySig+20)) {
        //       curMeasureArray[noteClass]=noteName;
        //}

        // we're done for a tied back note.
        return;
    }

    // check if current note needs acc
    if(typeof curMeasureArray[noteClass] !== 'undefined') {
        // we have information on the previous note
        // in the same measure:
        // if this note is the same noteClass and noteName
        // it doesn't need an accidental
        if(curMeasureArray[noteClass] == noteName) {
            // remove accidental if present
            if(note.accidental != null) {
                // security checks
                var thisPitch = note.pitch;
                var thisAcc = note.accidentalType;

                // remove
                note.accidentalType = Accidental.NONE;

                // if pitch changed, we were wrong...
                if(note.pitch != thisPitch) {
                    console.log("ERROR2: pitch of note changed!");
                    //note.color = "#ff0000";
                    note.accidentalType = thisAcc;
                }
            }
        }
    } else {
        // we don't have this note in the current measure
        // so it depends on the current key signature
        if(note.tpc > keySig+12 && note.tpc < keySig+20) {
            // we don't need an accidental in the current key sig
            // remove accidental if present
            if(note.accidental != null) {
                // security checks
                var thisPitch = note.pitch;
                var thisAcc = note.accidentalType;

                // remove
                note.accidentalType = Accidental.NONE;

                // if pitch changed, we were wrong...
                if(note.pitch != thisPitch) {
                    console.log("ERROR3: pitch of note changed!");
                    //note.color = "#ff0000";
                    note.accidentalType = thisAcc;
                    console.log("KeySig="+keySig+", tpc="+note.tpc);
                }
            }
        }
    }

    curMeasureArray[noteClass]=noteName;
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
        // we need to set staffIdx and voice to
        // get correct key signature.
        cursor.staffIdx = startTrack / 4;
        cursor.voice = 0;
        cursor.rewind(0);
    } else {
        cursor.rewind(1);
        // we need to set staffIdx and voice to
        // get correct key signature.
        cursor.staffIdx = startTrack / 4;
        cursor.voice = 0;
    }

    var segment = cursor.segment;

    // we use the cursor to know measure boundaries
    // and to get the current key signature
    var keySig = cursor.keySignature;
    cursor.nextMeasure();

    var curMeasureArray = [];

    // we use a segment, because the cursor always proceeds to
    // the next element in the given track and we don't know
    // in which track the element is.
    var inLastMeasure=false;
    while(segment && (processAll || segment.tick < endTick)) {
        // check if still inside same measure
        if(!inLastMeasure && !(segment.tick < cursor.tick)) {
            // new measure
            curMeasureArray = [];
            keySig = cursor.keySignature;
            if(!cursor.nextMeasure()) {
                inLastMeasure=true;
            }
        }

        for(var track=startTrack; track<endTrack; track++) {
            if(segment.elementAt(track) && segment.elementAt(track).type == Element.CHORD) {

                // process graceNotes if present
                if(segment.elementAt(track).graceNotes.length > 0) {
                    var graceChords = segment.elementAt(track).graceNotes;

                    for(var j=0;j<graceChords.length;j++) {
                        var notes = graceChords[j].notes;
                        for(var i=0;i<notes.length;i++) {
                            processNote(notes[i],curMeasureArray,keySig);
                        }
                    }
                }

                // process notes
                var notes = segment.elementAt(track).notes;

                for(var i=0;i<notes.length;i++) {
                    processNote(notes[i],curMeasureArray,keySig);
                }
            }
        }
        segment=segment.next;
    }
}

function removeAcc() {
    console.log("start remove courtesy accidentals");

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
        // to remove we need to parse staff by staff
        var curEndStaff = curStartStaff+1;

        // do the work
        processPart(cursor,endTick,curStartStaff*4,curEndStaff*4);

        // next part
        curStartStaff = curEndStaff;
    }

    console.log("end remove courtesy accidentals");
}



