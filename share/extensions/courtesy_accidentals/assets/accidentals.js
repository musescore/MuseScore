//==============================================
//  Cautionary Accidentals v4.0
//  https://github.com/XiaoMigros/Cautionary-Accidentals
//  Copyright (C)2023 XiaoMigros
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

function tpcToName(tpc) {
    var tpcNames = [ //-8 thru 40
        "Fbbb", "Cbbb", "Gbbb", "Dbbb", "Abbb", "Ebbb", "Bbbb",
        "Fbb",  "Cbb",  "Gbb",  "Dbb",  "Abb",  "Ebb",  "Bbb",
        "Fb",   "Cb",   "Gb",   "Db",   "Ab",   "Eb",   "Bb",
        "F",    "C",    "G",    "D",    "A",    "E",    "B",
        "F#",   "C#",   "G#",   "D#",   "A#",   "E#",   "B#",
        "F##",  "C##",  "G##",  "D##",  "A##",  "E##",  "B##"
    ]
    return tpcNames[tpc+8]
}

function lineFromClefType(cleftype) {
    // From engraving/dom/clef.cpp
    var pitchOffsets = [
        45, // ClefType.G
        31, // ClefType.G15_MB
        38, // ClefType.G8_VB
        52, // ClefType.G8_VA
        59, // ClefType.G15_MA
        38, // ClefType.G8_VB_O
        45, // ClefType.G8_VB_P
        47, // ClefType.G_1

        43, // ClefType.C1
        41, // ClefType.C2
        39, // ClefType.C3
        37, // ClefType.C4
        35, // ClefType.C5
        45, // ClefType.C_19C
        43, // ClefType.C1_F18C
        39, // ClefType.C3_F18C
        37, // ClefType.C4_F18C
        43, // ClefType.C1_F20C
        39, // ClefType.C3_F20C
        37, // ClefType.C4_F20C

        33, // ClefType.F
        19, // ClefType.F15_MB
        26, // ClefType.F8_VB
        40, // ClefType.F_8VA
        47, // ClefType.F_15MA
        35, // ClefType.F_B
        31, // ClefType.F_C
        33, // ClefType.F_F18C
        33, // ClefType.F_19C

        45, // ClefType.PERC
        45, // ClefType.PERC2

        45, // ClefType.TAB
        45, // ClefType.TAB4
        45, // ClefType.TAB_SERIF
        45, // ClefType.TAB4_SERIF

        30, // ClefType.C4_8VB
        38, // ClefType.G8_VB_C
    ]
    return pitchOffsets[cleftype];
}

function note2line(note) {
    return lineFromClefType(curScore.staves[note.vStaffIdx].clefType(note.fraction)) - note.line
}

function step2pitch(step)
{
    const tab = [0, 2, 4, 5, 7, 9, 11]
    return 12 * Math.floor(step / 7) + tab[step % 7]
}

function runPlugin(type) {
    curScore.startCmd("Courtesy accidentals")
    try {
        var full = false;
        if (!curScore.selection.elements.length) {
            console.log("No selection. Applying plugin to all notes...")
            cmd("select-all")
            full = true;
        } else {
            console.log("Applying plugin to selection")
        }
        switch (type) {
            case "add": {
                addCourtesyAccidentals()
                break;
            }
            case "remove": {
                removeCourtesyAccidentals()
                break;
            }
            default: console.warn("Unknown action requested.")
        }
        if (full) {
            curScore.selection.clear()
        }
        curScore.endCmd()
    } catch (e) {
        // If we encounter an error, rollback all changes
        curScore.endCmd(true)
        curScore.startCmd("Courtesy accidentals: " + e.toString())
        var text = newElement(Element.STAFF_TEXT)
        text.text = e.toString()
        var c = curScore.newCursor()
        c.track = 0
        c.rewindToFraction(fraction(0, 1))
        c.add(text)
        curScore.endCmd()
    }
    quit()
}

function isDoubleAccidental(accidentalType) {
    return accidentalType == Accidental.SHARP2 || accidentalType == Accidental.FLAT2 || accidentalType == Accidental.SHARP_SHARP
}

function isTripleAccidental(accidentalType) {
    return accidentalType == Accidental.SHARP3 || accidentalType == Accidental.FLAT3
}

function isNatDouble(accidentalType) {
    return accidentalType == Accidental.NATURAL_FLAT || accidentalType == Accidental.NATURAL_SHARP
}

function isDoubleTripleAccidental(accidentalType)
{
    return isDoubleAccidental(accidentalType) || isTripleAccidental(accidentalType) || isNatDouble(accidentalType)
}

function getReadableNotes()
{
    var notes = []
    curScore.selection.elements.filter((element) => element.staff.isPitchedStaff(element.fraction)).forEach((element) => {
        if (element.type == Element.NOTE) {
            let note = element
            notes.push({
                "scoreNote": note,
                "measure": note.parent.measure,
                "vStaffIdx": note.vStaffIdx,
                "accidentalType": note.accidentalType,
                "line": note2line(note),
                "grace": isGraceNote(note.parent),
                "ornament": false,
                "ornamentAccidental": false,
                "tick": note.fraction,
                "duration": isGraceNote(note.parent) ? fraction(0, 1) : note.parent.actualDuration,
                "pitch": note.pitch
            })
        } else if (element == Element.ORNAMENT) {
            // if (ornamentOption) {
                // continue
            // }
            let ornament = element

            if (ornament.hasIntervalAbove) {
                notes.push({
                    "scoreNote": false,
                    "vStaffIdx": ornament.vStaffIdx,
                    "measure": ornament.parent.measure,
                    "accidentalType": ornament.accidentalAbove.accidentalType,
                    "line": note2line(ornament.parent.upNote) - ornament.intervalAbove.step,
                    "grace": isGraceNote(ornament.parent),
                    "ornament": true,
                    "ornamentAccidental": false,
                    "tick": ornament.fraction,
                    "duration": isGraceNote(ornament.parent) ? fraction(0, 1) : ornament.parent.actualDuration,
                    "pitch": step2pitch(note2line(ornament.parent.upNote) - ornament.intervalAbove.step) + accidentalType2pitch(ornament.accidentalAbove.accidentalType)
                })
            }

            if (ornament.hasIntervalBelow) {
                notes.push({
                    "scoreNote": false,
                    "vStaffIdx": ornament.vStaffIdx,
                    "measure": ornament.parent.measure,
                    "accidentalType": ornament.accidentalBelow.accidentalType,
                    "line": note2line(ornament.parent.upNote) - ornament.intervalAbove.step,
                    "grace": isGraceNote(ornament.parent),
                    "ornament": true,
                    "ornamentAccidental": false,
                    "tick": ornament.fraction,
                    "duration": isGraceNote(ornament.parent) ? fraction(0, 1) : ornament.parent.actualDuration,
                    "pitch": step2pitch(note2line(ornament.parent.upNote) - ornament.intervalBelow.step) + accidentalType2pitch(ornament.accidentalBelow.accidentalType)
                })
            }
        }
    })
    return notes
}

function addCourtesyAccidentals() {
    if (options.uSettings && JSON.parse(options.uSettings).edited) {
        loadSettings(JSON.parse(options.uSettings))
    } else {
        loadSettings(DSettings.read())
    }
    const notes = getReadableNotes()

    // Exception: Only 1 note is selected
    if (notes.length == 1) {
        restateAccidental(notes[0], false, AccidentalBracket.NONE)
        return
    }

    notes.sort(function (a, b) {
        //sort notes by tick, prioritise notes with accidentals, prioritise non-doubles to avoid excessive brackets
        // @todo grace index?
        if (a.tick.ticks == b.tick.ticks) {
            var testCount = 0
            if (a.accidentalType != AccidentalType.NONE) {
                testCount--
            }
            if (b.accidentalType != AccidentalType.NONE) {
                testCount++
            }
            if (testCount == 0) {
                if (isDoubleTripleAccidental(a.accidentalType)) {
                    testCount++
                }
                if (isDoubleTripleAccidental(b.accidentalType)) {
                    testCount++//-- ??
                }
            }
            return testCount
        }
        return (a.tick.ticks - b.tick.ticks)
    })

    for (var i = notes.length-1; i >= 0; i--) {
        if (!notes[i].scoreNote) {
            continue
        }
        if (notes[i].scoreNote.accidental && notes[i].scoreNote.accidental.visible) {
            addAccidentals(notes.slice(i)) //notes.subarray non-functional
        } else if (setting7.addAccidentals || setting8.addAccidentals) {
            keySigTest(notes.slice(i))
        }
    }
}

// TODO: Note could be ornament fake, testNote not

function passTest1(note, testNote) {
    return setting1.addAccidentals && isSameNoteName(note, testNote) && !isSamePitch(note, testNote)
        && isSameMeasure(note, testNote) && isSameStaff(note, testNote) && (setting1.parseGraceNotes || !testNote.grace)
        && durationModeIsValid(setting1.durationMode, note, testNote)
}

function passTest2(note, testNote) {
    return setting2.addAccidentals && isSameNoteNameAndOctave(note, testNote) && !isSamePitch(note, testNote)
        && isSameMeasure(note, testNote)
        && !isSameStaff(note, testNote) && isSamePart(note, testNote) && (setting2.parseGraceNotes || !testNote.grace)
        && durationModeIsValid(setting2.durationMode, note, testNote)
}

function passTest3(note, testNote) {
    return setting3.addAccidentals && isSameNoteName(note, testNote) && !isSamePitch(note, testNote)
        && !isSameNoteNameAndOctave(note, testNote) && isSameMeasure(note, testNote) && !isSameStaff(note, testNote)
        && isSamePart(note, testNote) && (setting3.parseGraceNotes || !testNote.grace)
        && durationModeIsValid(setting3.durationMode, note, testNote)
}

function shouldCancel123(note, testNote, cancelledNotes) {
    for (var k in cancelledNotes) {
        if (isSameNoteName(note, cancelledNotes[k]) && isSamePitch(note, cancelledNotes[k]) && (setting9.a || isOctavedPitch(note, cancelledNotes[k]))
            && isSameStaff(note, cancelledNotes[k]) && isSameMeasure(note, cancelledNotes[k])) {
            console.log("The accidental in question has been cancelled, no need to add further cautionary accidentals")
            return false
        }
    }
    return true
}

function passTest4a(note, testNote) {
    return setting4.a.addAccidentals && isSameNoteName(note, testNote) && !isSamePitch(note, testNote)
        && (isNextMeasure(note, testNote) || isNextMeasure(note, testNote.lastTiedNote)) && isSameStaff(note, testNote) // lasttiednote
}

function shouldCancel4a(note, testNote, cancelledNotes) {
    for (var k in cancelledNotes) {
        if (isSameNoteName(note, cancelledNotes[k]) && isSamePitch(testNote, cancelledNotes[k]) && (setting9.a || isOctavedPitch(note, cancelledNotes[k]))
            && isSameStaff(note, cancelledNotes[k]) && isSameMeasure(note, cancelledNotes[k])) {
            console.log("The accidental in question has been cancelled, no need to add further cautionary accidentals")
            return false
        }
    }
    return true
}

function passTest5a(note, testNote) {
    return setting5.a.addAccidentals && isSameNoteName(note, testNote) && !isSamePitch(note, testNote)
        && (isNextMeasure(note, testNote) || isNextMeasure(note, testNote.lastTiedNote))
        && !isSameStaff(note, testNote) && isSamePart(note, testNote)
}

function shouldCancel5a(note, testNote, cancelledNotes) {
    for (var k in cancelledNotes) {
        if (isSameNoteName(note, cancelledNotes[k]) && isSamePitch(testNote, cancelledNotes[k]) && (setting9.b || isOctavedPitch(note, cancelledNotes[k]))
            && isSameStaff(note, cancelledNotes[k]) && isSameMeasure(note, cancelledNotes[k])) {
            console.log("The accidental in question has been cancelled, no need to add further cautionary accidentals")
            return false
        }
    }
    return true
}

function passTest6(note, testNote) {
    return setting6.a.addAccidentals && isSameNoteName(note, testNote) && isSamePitch(note, testNote)
        && testNote.grace && !note.grace && isSameMeasure(note, testNote)
        && (setting6.b.addAccidentals ? isSamePart(note, testNote) : isSameStaff(note, testNote))
}

function shouldCancel6(note, testNote, cancelledNotes) {
    for (var k in cancelledNotes) {
        if (isSameNoteName(note, cancelledNotes[k]) && isSamePitch(note, cancelledNotes[k]) && isSameStaff(note, cancelledNotes[k])) {
            //optional: change isSameStaff to (setting6.b.addAccidentals ? isSamePart(note, testNote) : isSameStaff(note, testNote))
            console.log("The accidental in question has been cancelled, no need to add further cautionary accidentals")
            return false
        }
    }
    return true
}

function passTest7(note, testNote) {
    return setting7.addAccidentals && isSameNoteName(note, testNote) && (setting7.cancelOctaves ? !isOctavedPitch(note, testNote) : (isSameNoteNameAndOctave(note, testNote) && !isSamePitch(note, testNote)))
        && note.accidentalType == Accidental.NONE && isNextMeasure(note, testNote) && isSameStaff(note, testNote) && (setting7.parseGraceNotes || !testNote.grace)
}

function shouldCancel7(note, testNote, cancelledNotes) {
    for (var k in cancelledNotes) {
        if (isSameNoteName(note, cancelledNotes[k]) && (setting7.cancelMode ? isOctavedPitch(note, cancelledNotes[k]) : isSamePitch(note, cancelledNotes[k])) &&
            isSameStaff(note, cancelledNotes[k])) {
            console.log("The accidental in question has been cancelled, no need to add further cautionary accidentals")
            return false
        }
    }
    return true
}

function passTest8(note, testNote) {
    return setting8.addAccidentals && isSameNoteName(note, testNote) && (setting8.cancelOctaves ? !isOctavedPitch(note, testNote) : (isSameNoteNameAndOctave(note, testNote) && !isSamePitch(note, testNote)))
        && note.accidentalType == Accidental.NONE && isSameMeasure(note, testNote) && isSameStaff(note, testNote) && (setting8.parseGraceNotes || !testNote.grace)
}

function shouldCancel8(note, testNote, cancelledNotes) {
    for (var k in cancelledNotes) {
        if (isSameNoteName(note, cancelledNotes[k]) && (setting8.cancelMode ? isOctavedPitch(note, cancelledNotes[k]) : isSamePitch(note, cancelledNotes[k])) &&
            isSameStaff(note, cancelledNotes[k])) {
            console.log("The accidental in question has been cancelled, no need to add further cautionary accidentals")
            return false
        }
    }
    return true
}

function addAccidentals(noteList) {
    var testNote = noteList.shift()
    var testName = tpcToNote(testNote.tpc)
    console.log("Note with accidental found (" //+ tpcToName(testNote.tpc) + ").\r\n"
        + "Attempting to add cautionary accidentals to " + noteList.length + " note(s).")
    var cancelledNotes = []
    for (var j in noteList) {
        var note = noteList[j]
        var changeBracket = 7
        if (!note.scoreNote || !note.scoreNote.tieBack) {
            if (passTest1(note, testNote) && shouldCancel123(note, testNote, cancelledNotes)) {
                changeBracket = Math.min(changeBracket, setting1.bracketType)
                if (isSameNoteName(note, testNote) && isSameStaff(note, testNote)) {
                    //isSameStaff might not be needed here
                    cancelledNotes.push(note)
                }
            }

            if (passTest2(note, testNote) && shouldCancel123(note, testNote, cancelledNotes)) {
                changeBracket = Math.min(changeBracket, setting2.bracketType)
                if (isSameNoteName(note, testNote) && isSamePart(note, testNote)) {
                    //isSamePart might not be needed here
                    cancelledNotes.push(note)
                }
            }

            if (passTest3(note, testNote) && shouldCancel123(note, testNote, cancelledNotes)) {
                changeBracket = Math.min(changeBracket, setting3.bracketType)
                if (isSameNoteName(note, testNote) && isSamePart(note, testNote)) {
                    //isSamePart might not be needed here
                    cancelledNotes.push(note)
                }
            }

            if (passTest4a(note, testNote) && shouldCancel4a(note, testNote, cancelledNotes)) {
                if (isSameNoteNameAndOctave(note, testNote) && (setting4.parseGraceNotes || !testNote.grace)) {
                    changeBracket = Math.min(changeBracket, setting4.a.bracketType)
                } else if (setting4.b.addAccidentals && (setting4.b.parseGraceNotes || !testNote.grace)) {
                    changeBracket = Math.min(changeBracket, setting4.b.bracketType)
                }
                if (isSameNoteName(note, testNote) && isSameStaff(note, testNote)) {
                    //isSameStaff might not be needed here
                    cancelledNotes.push(note)
                }
            }

            if (passTest5a(note, testNote) && shouldCancel5a(note, testNote, cancelledNotes)) {
                if (isSameNoteNameAndOctave(note, testNote) && (setting5.a.parseGraceNotes || !testNote.grace)) {
                    changeBracket = Math.min(changeBracket, setting5.a.bracketType)
                } else if (setting5.b.addAccidentals && (setting5.b.parseGraceNotes || !testNote.grace)) {
                    changeBracket = Math.min(changeBracket, setting5.b.bracketType)
                }
                if (isSameNoteName(note, testNote) && isSamePart(note, testNote)) {
                    //isSamePart might not be needed here
                    cancelledNotes.push(note)
                }
            }

            if (passTest6(note, testNote)) {
                cancelledNotes.push(note)
                if (isSameStaff(note, testNote)) {
                    changeBracket = Math.min(changeBracket, setting6.a.bracketType)
                } else {
                    changeBracket = Math.min(changeBracket, setting6.b.bracketType)
                }
            }

            if (changeBracket != 7) {
                if (isSameTick(note, testNote) && (testNote.tpc > 26 || testNote.tpc < 6)) {
                    changeBracket = Math.min(changeBracket, 0) //dont add brackets to reduced accidentals on same beat //TODO: same measure?
                }
                restateAccidental(note, shouldCancelDouble(testNote), changeBracket)
                if (isSameNoteNameAndOctave(note, testNote)) {
                    cancelledNotes.push(note)
                    //only stop adding cautionary accidentals if note is of the same octave
                }
            }
        }
    }
}

function keySigTest(noteList) {
    var testNote = noteList.shift()
    var testName = tpcToNote(testNote.tpc)
    console.log("Testing for key signature changes")
    var cancelledNotes = []
    for (var j in noteList) {
        var note = noteList[j]
        var changeBracket = 7
        if (note.scoreNote && !note.scoreNote.tieBack) {
            if (passTest7(note, testNote) && shouldCancel7(note, testNote, cancelledNotes)) {
                changeBracket = Math.min(changeBracket, setting7.bracketType)
                if (isSameNoteName(note, testNote) && isSameStaff(note, testNote) && (!setting7.cancelMode || isSameNoteNameAndOctave(note, testNote))) {
                    cancelledNotes.push(note)
                }
            }

            if (passTest8(note, testNote) && shouldCancel8(note, testNote, cancelledNotes)) {
                changeBracket = Math.min(changeBracket, setting8.bracketType)
                if (isSameNoteName(note, testNote) && isSameStaff(note, testNote) && (!setting8.cancelMode || isSameNoteNameAndOctave(note, testNote))) {
                    cancelledNotes.push(note)
                }
            }

            if (changeBracket != 7) {
                if (isSameTick(note, testNote) && (testNote.tpc > 26 || testNote.tpc < 6)) {
                    changeBracket = Math.min(changeBracket, 0)
                }
                restateAccidental(note, shouldCancelDouble(testNote), changeBracket)
            }
        }
    }
}

function loadSettings(settingObj) {
    setting0 = settingObj.setting0
    setting1 = settingObj.setting1
    setting2 = settingObj.setting2
    setting3 = settingObj.setting3
    setting4 = settingObj.setting4
    setting5 = settingObj.setting5
    setting6 = settingObj.setting6
    setting7 = settingObj.setting7
    setting8 = settingObj.setting8
    setting9 = settingObj.setting9
}

function tpcToNote(tpc) {
    var noteNames = ["C", "G", "D", "A", "E", "B", "F"]
    return noteNames[(tpc+14) % 7]
}

function removeCourtesyAccidentals() {
    curScore.selection.elements.filter((element) => element.type == Element.NOTE).forEach((note) => {
        destateAccidental(note)
    })
}

function accidentalType2pitch(accidentalType) {
    switch (accidentalType) {
    case AccidentalType.SHARP3:
        return AccidentalVal.SHARP3
    case AccidentalType.SHARP2:
        return AccidentalVal.SHARP2
    case AccidentalType.SHARP:
        return AccidentalVal.SHARP
    case AccidentalType.NATURAL:
        return AccidentalVal.NATURAL
    case AccidentalType.FLAT:
        return AccidentalType.FLAT
    case AccidentalType.FLAT2:
        return AccidentalVal.FLAT2
    case AccidentalType.FLAT3:
        return AccidentalVal.FLAT3
    default:
        break
    }
    return AccidentalVal.NATURAL
}

function isGraceNote(note) {
    return note.noteType != NoteType.NORMAL
}

function isSameNoteName(note1, note2) {
    // return tpcToNote(note1.tpc) == tpcToNote(note2.tpc)
    return note1.line % 7 == note2.line % 7
}

function isSamePitch(note1, note2) {
    return note1.pitch == note2.pitch
}

function isSameNoteNameAndOctave(note1, note2) {
    dbg = "isSameNoteNameAndOctave"
    return note1.line == note2.line && Math.abs(note1.line - note2.line) < 4 // this accounts for clefs currently, does it need to?
}

function isOctavedPitch(note1, note2) {
    return note1.pitch % 12 == note2.pitch % 12
}

function isSameTick(note1, note2) {
    return note1.tick.equals(note2.tick)
}

function isSameBeat(note1, note2) {
    return note1.tick.lessThan(note2.tick.plus(note2.duration))
}

function isSameMeasure(note1, note2) {
    return note1.measure.is(note2.measure)
}

function isNextMeasure(note1, note2) {  // order is relevant here
    return note1.measure.is(curScore.firstMeasure) ? false : note1.measure.prevMeasure.is(note2.measure)
}

function isSameStaff(note1, note2) {
    return note1.vStaffIdx == note2.vStaffIdx
}

function isSamePart(note1, note2) {
    return curScore.staves[note1.vStaffIdx].part.is(curScore.staves[note2.vStaffIdx].part)
}

function durationModeIsValid(durationMode, note, testNote) {  // order is relevant here
    return durationMode == 0 || (durationMode == 1 && isSameTick(note, testNote)) || (durationMode == 2 && isSameBeat(note, testNote))
}

// @todo account for ornaments here
function shouldCancelDouble(note) {
    return (note.scoreNote && setting0.addNaturals) ? (note.scoreNote.tpc > 26 || note.scoreNote.tpc < 6) : false
}

function restateAccidental(n, cancelDouble, bracketType) {
    if (!n.scoreNote) {
        return
    }
    var note = n.scoreNote
    var oldAccidental = note.accidentalType
    var accidental = Accidental.NONE
    if (note.tpc > 33) {
        accidental = Accidental.SHARP3
    } else if (note.tpc > 26) {
        accidental = Accidental.SHARP2
    } else if (note.tpc > 19) {
        accidental = cancelDouble ? Accidental.NATURAL_SHARP : Accidental.SHARP
    } else if (note.tpc > 12) {
        accidental = Accidental.NATURAL
    } else if (note.tpc > 5) {
        accidental = cancelDouble ? Accidental.NATURAL_FLAT : Accidental.FLAT
    } else if (note.tpc > -2) {
        accidental = Accidental.FLAT2
    } else {
        accidental = Accidental.FLAT3
    }
    if (accidental != oldAccidental) {
        note.accidentalType = accidental
        note.accidental.visible = note.visible
        note.accidental.accidentalBracket = bracketType
        console.log("Added a cautionary accidental to note " + tpcToName(note.tpc))
    }
}

function destateAccidental(note) {
    if (note.accidental) {
        var oldAccidental = note.accidentalType
        if (note.accidentalType == Accidental.NATURAL_FLAT) {
            oldAccidental = Accidental.FLAT
        }
        if (note.accidentalType == Accidental.NATURAL_SHARP) {
            oldAccidental = Accidental.SHARP
        }
    }
    var oldPitch = note.pitch
    note.accidentalType = Accidental.NONE
    if (note.pitch != oldPitch) {
        note.accidentalType = oldAccidental
        console.log("Keeping existing accidental for note " + tpcToName(note.tpc))
    } else {
        console.log("Removing accidental from note " + tpcToName(note.tpc))
    }
}
