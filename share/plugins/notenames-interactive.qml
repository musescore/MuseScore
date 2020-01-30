//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Note Names Plugin
//
//  Copyright (C) 2012 Werner Schweer
//  Copyright (C) 2013 - 2019 Joachim Schmitz
//  Copyright (C) 2014 Jörn Eichler
//  Copyright (C) 2020 MuseScore BVBA
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

import QtQuick 2.2
import QtQuick.Controls 2.0
import MuseScore 3.0

MuseScore {
    version: "3.4"
    description: qsTr("This plugin names notes as per your language setting")
    menuPath: "Plugins.Notes." + qsTr("Note Names (Interactive)")
    pluginType: "dock"

    implicitHeight: controls.implicitHeight * 1.5
    implicitWidth: controls.implicitWidth

    // Small note name size is fraction of the full font size.
    property var defaultFontSize
    property var fontSizeMini: 0.7;

    property int nstaves: 0 // for validators in staff number inputs

    property bool inCmd: false

    function ensureCmdStarted() {
        if (!inCmd) {
            curScore.startCmd();
            inCmd = true;
        }
    }

    function ensureCmdEnded() {
        if (inCmd) {
            curScore.endCmd();
            inCmd = false;
        }
    }

    function findSegment(el) {
        while (el && el.type != Element.SEGMENT)
            el = el.parent;
        return el;
    }

    function getChordName(chord) {
        var text = "";
        var notes = chord.notes;
        for (var i = 0; i < notes.length; i++) {
            var sep = "\n";   // change to "," if you want them horizontally (anybody?)
            if ( i > 0 )
                text = sep + text; // any but top note
            if (typeof notes[i].tpc === "undefined") // like for grace notes ?!?
                return;
            switch (notes[i].tpc) {
                case -1: text = qsTranslate("InspectorAmbitus", "F♭♭") + text; break;
                case  0: text = qsTranslate("InspectorAmbitus", "C♭♭") + text; break;
                case  1: text = qsTranslate("InspectorAmbitus", "G♭♭") + text; break;
                case  2: text = qsTranslate("InspectorAmbitus", "D♭♭") + text; break;
                case  3: text = qsTranslate("InspectorAmbitus", "A♭♭") + text; break;
                case  4: text = qsTranslate("InspectorAmbitus", "E♭♭") + text; break;
                case  5: text = qsTranslate("InspectorAmbitus", "B♭♭") + text; break;
                case  6: text = qsTranslate("InspectorAmbitus", "F♭")  + text; break;
                case  7: text = qsTranslate("InspectorAmbitus", "C♭")  + text; break;

                case  8: text = qsTranslate("InspectorAmbitus", "G♭")  + text; break;
                case  9: text = qsTranslate("InspectorAmbitus", "D♭")  + text; break;
                case 10: text = qsTranslate("InspectorAmbitus", "A♭")  + text; break;
                case 11: text = qsTranslate("InspectorAmbitus", "E♭")  + text; break;
                case 12: text = qsTranslate("InspectorAmbitus", "B♭")  + text; break;
                case 13: text = qsTranslate("InspectorAmbitus", "F")   + text; break;
                case 14: text = qsTranslate("InspectorAmbitus", "C")   + text; break;
                case 15: text = qsTranslate("InspectorAmbitus", "G")   + text; break;
                case 16: text = qsTranslate("InspectorAmbitus", "D")   + text; break;
                case 17: text = qsTranslate("InspectorAmbitus", "A")   + text; break;
                case 18: text = qsTranslate("InspectorAmbitus", "E")   + text; break;
                case 19: text = qsTranslate("InspectorAmbitus", "B")   + text; break;

                case 20: text = qsTranslate("InspectorAmbitus", "F♯")  + text; break;
                case 21: text = qsTranslate("InspectorAmbitus", "C♯")  + text; break;
                case 22: text = qsTranslate("InspectorAmbitus", "G♯")  + text; break;
                case 23: text = qsTranslate("InspectorAmbitus", "D♯")  + text; break;
                case 24: text = qsTranslate("InspectorAmbitus", "A♯")  + text; break;
                case 25: text = qsTranslate("InspectorAmbitus", "E♯")  + text; break;
                case 26: text = qsTranslate("InspectorAmbitus", "B♯")  + text; break;
                case 27: text = qsTranslate("InspectorAmbitus", "F♯♯") + text; break;
                case 28: text = qsTranslate("InspectorAmbitus", "C♯♯") + text; break;
                case 29: text = qsTranslate("InspectorAmbitus", "G♯♯") + text; break;
                case 30: text = qsTranslate("InspectorAmbitus", "D♯♯") + text; break;
                case 31: text = qsTranslate("InspectorAmbitus", "A♯♯") + text; break;
                case 32: text = qsTranslate("InspectorAmbitus", "E♯♯") + text; break;
                case 33: text = qsTranslate("InspectorAmbitus", "B♯♯") + text; break;
                default: text = qsTr("?")   + text; break;
            } // end switch tpc

            // octave, middle C being C4
            //text += (Math.floor(notes[i].pitch / 12) - 1)
            // or
            //text += (Math.floor(notes[i].ppitch / 12) - 1)

            // change below false to true for courtesy- and microtonal accidentals
            // you might need to come up with suitable translations
            // only #, b, natural and possibly also ## seem to be available in UNICODE
            if (false) {
                switch (notes[i].userAccidental) {
                    case  0: break;
                    case  1: text = qsTranslate("accidental", "Sharp") + text; break;
                    case  2: text = qsTranslate("accidental", "Flat") + text; break;
                    case  3: text = qsTranslate("accidental", "Double sharp") + text; break;
                    case  4: text = qsTranslate("accidental", "Double flat") + text; break;
                    case  5: text = qsTranslate("accidental", "Natural") + text; break;
                    case  6: text = qsTranslate("accidental", "Flat-slash") + text; break;
                    case  7: text = qsTranslate("accidental", "Flat-slash2") + text; break;
                    case  8: text = qsTranslate("accidental", "Mirrored-flat2") + text; break;
                    case  9: text = qsTranslate("accidental", "Mirrored-flat") + text; break;
                    case 10: text = qsTranslate("accidental", "Mirrored-flat-slash") + text; break;
                    case 11: text = qsTranslate("accidental", "Flat-flat-slash") + text; break;
                    case 12: text = qsTranslate("accidental", "Sharp-slash") + text; break;
                    case 13: text = qsTranslate("accidental", "Sharp-slash2") + text; break;
                    case 14: text = qsTranslate("accidental", "Sharp-slash3") + text; break;
                    case 15: text = qsTranslate("accidental", "Sharp-slash4") + text; break;
                    case 16: text = qsTranslate("accidental", "Sharp arrow up") + text; break;
                    case 17: text = qsTranslate("accidental", "Sharp arrow down") + text; break;
                    case 18: text = qsTranslate("accidental", "Sharp arrow both") + text; break;
                    case 19: text = qsTranslate("accidental", "Flat arrow up") + text; break;
                    case 20: text = qsTranslate("accidental", "Flat arrow down") + text; break;
                    case 21: text = qsTranslate("accidental", "Flat arrow both") + text; break;
                    case 22: text = qsTranslate("accidental", "Natural arrow down") + text; break;
                    case 23: text = qsTranslate("accidental", "Natural arrow up") + text; break;
                    case 24: text = qsTranslate("accidental", "Natural arrow both") + text; break;
                    case 25: text = qsTranslate("accidental", "Sori") + text; break;
                    case 26: text = qsTranslate("accidental", "Koron") + text; break;
                    default: text = qsTr("?") + text; break;
                }  // end switch userAccidental
            }  // end if courtesy- and microtonal accidentals
        }  // end for note

        return text;
    }

    function getGraceNoteNames(graceChordsList) {
        var names = [];
        // iterate through all grace chords
        for (var chordNum = 0; chordNum < graceChordsList.length; chordNum++) {
            var chord = graceChordsList[chordNum];
            var chordName = getChordName(chord);
            // append the name to the list of names
            names.push(chordName);

        }
        return names;
    }

    function getAllChords(el) {
        // List chords in the following order:
        // 1) Leading grace notes;
        // 2) Chord itself;
        // 3) Trailing grace notes.

        if (!el || el.type != Element.CHORD)
            return [];

        var chord = el;
        var allChords = [ chord ];

        // Search for grace notes
        var graceChords = chord.graceNotes;
        for (var chordNum = 0; chordNum < graceChords.length; chordNum++) {
            var graceChord = graceChords[chordNum];
            var noteType = graceChord.noteType;

            switch (noteType) {
                case NoteType.GRACE8_AFTER:
                case NoteType.GRACE16_AFTER:
                case NoteType.GRACE32_AFTER:
                    leadingLifo.push(graceChord); // append trailing grace chord to list
                    break;
                default:
                    allChords.unshift(graceChord); // prepend leading grace chord to list
                    break;
            }
        }

        return allChords;
    }

    function isNoteName(el) {
        return el.type == Element.STAFF_TEXT; // TODO: how to distinguish note names from all staff texts?
    }

    function getExistingNoteNames(segment, track) {
        var annotations = segment.annotations;
        var noteNames = [];

        for (var i = 0; i < annotations.length; ++i) {
            var a = annotations[i];
            if (a.track != track)
                continue;
            if (isNoteName(a))
                noteNames.push(a);
        }

        return noteNames;
    }

    function handleChordAtCursor(cursor) {
        var allNoteNames = getExistingNoteNames(cursor.segment, cursor.track);
        var allChords = getAllChords(cursor.element);
        var chordIdx = 0;

        for (; chordIdx < allChords.length; ++chordIdx) {
            var chord = allChords[chordIdx];
            var noteName = allNoteNames[chordIdx];

            var chordProperties = {
                "offsetX": chord.posX,
                "fontSize" : chord.noteType == NoteType.NORMAL ? defaultFontSize : (defaultFontSize * fontSizeMini),
                "placement": (chord.voice & 1) ? Placement.BELOW : Placement.ABOVE, // place below for voice 1 and voice 3 (numbered as 2 and 4 in user interface)
                "text": getChordName(chord)
            }

            if (!noteName) {
                // Note name does not exist, add a new one
                ensureCmdStarted();
                var nameText = newElement(Element.STAFF_TEXT);
                for (var prop in chordProperties) {
                    if (nameText[prop] != chordProperties[prop])
                        nameText[prop] = chordProperties[prop];
                }
                cursor.add(nameText);
            } else {
                // Note name exists, ensure it is up to date
                for (var prop in chordProperties) {
                    if (noteName[prop] != chordProperties[prop]) {
                        ensureCmdStarted();
                        noteName[prop] = chordProperties[prop];
                    }
                }
            } // end if/else noteName
        } // end for allChords

        // Remove the remaining redundant note names, if any
        for (; chordIdx < allNoteNames.length; ++chordIdx) {
            ensureCmdStarted();
            var noteName = allNoteNames[chordIdx];
            removeElement(noteName);
        }
    } // end handleChordAtCursor()

    function processRange(startTick, endTick, firstStaff, lastStaff) {
        if (startTick < 0)
            startTick = 0;
        if (endTick < 0)
            endTick = Infinity; // process the entire score

        var cursor = curScore.newCursor();

        for (var staff = firstStaff; staff <= lastStaff; staff++) {
            for (var voice = 0; voice < 4; voice++) {
                cursor.voice    = voice;
                cursor.staffIdx = staff;
                cursor.rewindToTick(startTick);

                while (cursor.segment && cursor.tick <= endTick) {
                    handleChordAtCursor(cursor);
                    cursor.next();
                } // end while segment

            } // end for voice
        } // end for staff

        ensureCmdEnded();
    }

    function getStavesRange() {
        if (allStavesCheckBox.checked)
            return [0, curScore.nstaves];

        var firstStaff = firstStaffInput.acceptableInput ? +firstStaffInput.text : curScore.nstaves;
        var lastStaff = lastStaffInput.acceptableInput ? +lastStaffInput.text : -1;

        return [firstStaff, lastStaff];
    }

    onScoreStateChanged: {
        if (inCmd) // prevent recursion from own changes
            return;
        if (state.undoRedo) // try not to interfere with undo/redo commands
            return;

        nstaves = curScore.nstaves; // needed for validators in staff number inputs

        if (!noteNamesEnabledCheckBox.checked)
            return;
        if (!curScore || state.startLayoutTick < 0) // nothing to process?
            return;

        var stavesRange = getStavesRange();

        processRange(state.startLayoutTick, state.endLayoutTick, stavesRange[0], stavesRange[1]);
    }

    onRun: {
        defaultFontSize = newElement(Element.STAFF_TEXT).fontSize;
    }

    Column {
        id: controls

        CheckBox {
            id: noteNamesEnabledCheckBox
            text: "Enable notes naming"
        }
        CheckBox {
            id: allStavesCheckBox
            checked: true
            text: "All staves"
        }

        Grid {
            id: staffRangeControls
            columns: 2
            spacing: 4
            enabled: !allStavesCheckBox.checked

            Text {
                height: firstStaffInput.height
                verticalAlignment: Text.AlignVCenter
                text: "first staff:"
            }
            TextField {
                id: firstStaffInput
                text: "0"
                validator: IntValidator { bottom: 0; top: nstaves - 1 }
                onTextChanged: {
                    if (+lastStaffInput.text < +text)
                        lastStaffInput.text = text
                }
            }

            Text {
                height: lastStaffInput.height
                verticalAlignment: Text.AlignVCenter
                text: "last staff:"
            }
            TextField {
                id: lastStaffInput
                text: "0"
                validator: IntValidator { bottom: 0; top: nstaves - 1 }
                onTextChanged: {
                    if (text !== "" && (+firstStaffInput.text > +text))
                        firstStaffInput.text = text
                }
            }
        }
    }
}
