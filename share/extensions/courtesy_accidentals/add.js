//==============================================
//  add courtesy accidentals v1.0
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
    addAcc()
    curScore.endCmd()

    quit()
}

// configuration
// This has changed for MuseScore v3
// 0 = no bracket, 1 = parenthesis, 2 = bracket
var useBracket = 0

// if nothing is selected process whole score
var processAll = false

// function tpcName
//
// return name of note
const TPC_NAMES = [
                    "Fbb", "Cbb", "Gbb", "Dbb", "Abb", "Ebb", "Bbb",
                    "Fb",   "Cb",   "Gb",   "Db",   "Ab",   "Eb",   "Bb",
                    "F",     "C",     "G",     "D",     "A",     "E",     "B",
                    "F#",   "C#",   "G#",   "D#",   "A#",    "E#",   "B#",
                    "F##", "C##", "G##", "D##", "A##",  "E##",  "B##"
                ];

function tpcName(tpc) {
    return(TPC_NAMES[tpc+1]);
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

function processNote(note,prevMeasureArray,curMeasureArray) {
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
    curMeasureArray[noteClass]=noteName;

    // check if current note needs courtesy acc
    if(typeof prevMeasureArray[noteClass] !== 'undefined') {
        if(prevMeasureArray[noteClass] != noteName) {
            // this note needs an accidental
            // if there's none present anyway
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
                // put bracket on accidental
                note.accidental.accidentalBracket = useBracket;
            }
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

    var segment = cursor.segment;

    // we use the cursor to know measure boundaries
    cursor.nextMeasure();

    var curMeasureArray = [];
    var prevMeasureArray = [];

    // we use a segment, because the cursor always proceeds to
    // the next element in the given track and we don't know
    // in which track the element is.
    var inLastMeasure=false;
    while(segment && (processAll || segment.tick < endTick)) {
        // check if still inside same measure
        if(!inLastMeasure && !(segment.tick < cursor.tick)) {
            // new measure
            prevMeasureArray = curMeasureArray;
            curMeasureArray = [];
            if(!cursor.nextMeasure()) {
                inLastMeasure=true;
            }
        }

        // we search for key signatures in first voice of
        // first staff:
        var keySigTrack = startTrack - (startTrack % 4);

        for(var track=startTrack; track<endTrack; track++) {
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

            if(segment.elementAt(keySigTrack)
                    && segment.elementAt(keySigTrack).type == Element.KEYSIG
                    && (!segment.elementAt(keySigTrack).generated)) {
                //console.log("found KEYSIG");
                // just forget the previous measure info
                // to not generate any courtesy accidentals
                prevMeasureArray = [];
            }
            // look for notes and grace notes
            if(segment.elementAt(track) && segment.elementAt(track).type == Element.CHORD) {
                // process graceNotes if present
                if(segment.elementAt(track).graceNotes.length > 0) {
                    var graceChords = segment.elementAt(track).graceNotes;

                    for(var j=0;j<graceChords.length;j++) {
                        var notes = graceChords[j].notes;
                        for(var i=0;i<notes.length;i++) {
                            processNote(notes[i],prevMeasureArray,curMeasureArray);
                        }
                    }
                }

                // process notes
                var notes = segment.elementAt(track).notes;

                for(var i=0;i<notes.length;i++) {
                    processNote(notes[i],prevMeasureArray,curMeasureArray);
                }
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

    console.log("end add courtesy accidentals");
}



