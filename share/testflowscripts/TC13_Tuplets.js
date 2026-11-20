/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

var NewScore = require("steps/NewScore.js")
var NoteInput = require("steps/NoteInput.js")
var Score = require("steps/Score.js")
var Home = require("steps/Home.js")
var Project = require("steps/Project.js")
var Commands = require("steps/Commands.js")
var ScoreContent = require("steps/ScoreContent.js")
var engraving = require("MuseApi.Engraving")
var Tuplets = require("steps/Tuplets.js")
var Parts = require("steps/Parts.js")
var ScoreState = require("steps/ScoreState.js")
var UndoRedo = require("steps/UndoRedo.js")

const MEASURE_TICKS = 1920 // 4/4
const QUARTER_TICKS = 480

//! NOTE How many measures of quarter notes the set-up enters; the cases each work on their own measure
const NOTE_MEASURES = 8
const QUARTERS = [
    Commands.ENTER_NOTE_C_COMMAND, Commands.ENTER_NOTE_D_COMMAND,
    Commands.ENTER_NOTE_E_COMMAND, Commands.ENTER_NOTE_F_COMMAND
]

//! NOTE The measure with the 16th note, then measures with only a measure rest
const SIXTEENTH_MEASURE = NOTE_MEASURES
const REST_MEASURE = NOTE_MEASURES + 1

const ADD_DUPLET_COMMAND = "command://notation/add-tuplet/duplet"
const ADD_TRIPLET_COMMAND = "command://notation/add-tuplet/triplet"
const ADD_QUINTUPLET_COMMAND = "command://notation/add-tuplet/quintuplet"
const ADD_NEXT_CHORD_TO_SELECTION_COMMAND = "command://notation/add-to-selection/next-chord"
//! NOTE The first measure the note input cases use: empty measures after the measure-rest cases
const NOTE_INPUT_MEASURE = REST_MEASURE + 1
//! NOTE An empty measure to paste into
const PASTE_MEASURE = NOTE_INPUT_MEASURE + 4

const DELETE_COMMAND = "command://notation/delete"
const HALVE_DURATION_COMMAND = "command://notation/change-duration/halve"
const DOUBLE_DURATION_COMMAND = "command://notation/change-duration/double"
const COPY_COMMAND = "command://notation/copy"
//! NOTE The last measure the cases change
const LAST_MEASURE = PASTE_MEASURE
const FILE_NAME = "TC13_Tuplets.mscz"
const PASTE_COMMAND = "command://notation/paste"

//! NOTE The tuplet command with its fields, as the Other… dialog and the options send it
function addTupletCommand(fields)
{
    return "command://notation/add-tuplet?" + fields
}

//! NOTE Puts the keyboard focus on the score, then selects the note or rest at beat `beat` (0 = the first)
//! of measure `measureIndex` on the flute staff
function selectAt(measureIndex, beat)
{
    Score.focusIn()
    ScoreContent.selectAt(0, measureIndex * MEASURE_TICKS + (beat || 0) * QUARTER_TICKS)
    api.testflow.sleep(300)
}

//! NOTE Selects beats `startBeat` up to (not including) `endBeat` of a measure on the flute staff, the way a
//! user does: the first note, then "add next chord to selection" (Selection.selectRange() sets the range but
//! not the list of selected notes that commands work from)
function selectRange(measureIndex, startBeat, endBeat)
{
    var startTick = measureIndex * MEASURE_TICKS + startBeat * QUARTER_TICKS
    var endTick = measureIndex * MEASURE_TICKS + endBeat * QUARTER_TICKS
    selectAt(measureIndex, startBeat)
    var sel = engraving.curScore.selection
    for (var i = 0; i < 16 && !(sel.isRange && sel.endSegment && sel.endSegment.tick >= endTick); ++i) {
        api.dispatcher.dispatch(ADD_NEXT_CHORD_TO_SELECTION_COMMAND)
        api.testflow.sleep(100)
        sel = engraving.curScore.selection
    }
    if (!sel.isRange || sel.startSegment.tick !== startTick || sel.endSegment.tick !== endTick) {
        api.testflow.fatal("could not select ticks " + startTick + "-" + endTick)
    }
}

//! NOTE `count` copies of `text`, separated by spaces
function repeated(text, count)
{
    var parts = []
    for (var i = 0; i < count; ++i) {
        parts.push(text)
    }
    return parts.join(" ")
}

//! NOTE Turns note input on or off with an input method ("by-note-name", "by-duration", "rhythm"),
//! waiting until it's in that state
function setNoteInput(method, on)
{
    var command = "command://notation/toggle-note-input/" + method
    if (api.testflow.isCommandChecked(command) !== on) {
        api.dispatcher.dispatch(command)
    }
    for (var waited = 0; waited < 5000; waited += 50) {
        if (api.testflow.isCommandChecked(command) === on) {
            api.testflow.sleep(100)
            return
        }
        api.testflow.sleep(50)
    }
    api.testflow.fatal("note input " + method + " is not " + (on ? "on" : "off"))
}

//! NOTE The rhythm of every measure the cases use, on the flute staff of `score`
function allRhythms(score)
{
    var result = []
    for (var m = 0; m <= LAST_MEASURE; ++m) {
        result.push("measure " + (m + 1) + ": " + Tuplets.rhythm(m, 0, score))
    }
    return result
}

//! NOTE Everything the end checks compare: the content lines and rhythms of the score and its parts
function snapshotAll()
{
    var result = { score: ScoreContent.contentLines(engraving.curScore), rhythms: allRhythms(), parts: [] }
    var parts = Parts.list()
    for (var i = 0; i < parts.length; ++i) {
        result.parts.push({ title: parts[i].title, lines: ScoreContent.contentLines(parts[i].score),
                            rhythms: allRhythms(parts[i].score) })
    }
    return result
}

function expectSameSnapshot(expected, actual, what)
{
    ScoreContent.expectSameLines(expected.score, actual.score, what + ", score")
    ScoreContent.expectSameLines(expected.rhythms, actual.rhythms, what + ", rhythms")
    if (actual.parts.length !== expected.parts.length) {
        api.testflow.fatal(what + ": expected " + expected.parts.length + " parts, got " + actual.parts.length)
    }
    for (var i = 0; i < expected.parts.length; ++i) {
        ScoreContent.expectSameLines(expected.parts[i].lines, actual.parts[i].lines, what + ", part " + expected.parts[i].title)
        ScoreContent.expectSameLines(expected.parts[i].rhythms, actual.parts[i].rhythms,
                                     what + ", part " + expected.parts[i].title + " rhythms")
    }
}

//! NOTE Waits until a project is open (loading may take a moment)
function waitForProject(timeoutMsec)
{
    for (var waited = 0; waited < timeoutMsec; waited += 100) {
        if (Project.isOpened() && engraving.curScore) {
            return true
        }
        api.testflow.sleep(100)
    }
    return Project.isOpened() && engraving.curScore
}

//! NOTE State remembered by one step and checked by a later one
var initialState = null
var finalState = null
var savedState = null

var testCase = {
    name: "TC13: Tuplets",
    description: "Tuplets by the tuplet commands, in normal mode and note input, with their options; checks each measure's rhythm",
    steps: [
        {name: "Close score (if opened) and go to home to start", func: function() {
            Project.closeIfOpened()
            Home.goToHome()
        }},
        {name: "Create score with Flute and Oboe", func: function() {
            NewScore.openNewScoreDialog()
            NewScore.chooseInstrument("Woodwinds", "Flute")
            api.testflow.seeChanges()
            //! NOTE Woodwinds is already selected, so pick Oboe from the same list
            NewScore.selectInstrument("Oboe")
            //! NOTE Adding an instrument is not instant, give it time before pressing Done
            api.testflow.sleep(1000)
            NewScore.done()
            if (!waitForProject(5000) || engraving.curScore.parts.length !== 2) {
                api.testflow.fatal("create score: expected a score with 2 instruments")
            }
        }},
        {name: "Open all parts, then back to the score", func: function() {
            // opened first, so every edit below also has to reach the parts
            Parts.openAll()
            Parts.goToMainScore()
            //! NOTE Parts may take a moment to be created
            for (var waited = 0; Parts.list().length < 2 && waited < 5000; waited += 200) {
                api.testflow.sleep(200)
            }
            if (Parts.list().length !== 2) {
                api.testflow.fatal("open parts: expected 2 parts, got " + Parts.list().length)
            }
            initialState = ScoreState.snapshot()
        }},
        {name: "Put quarter notes in the first measures", func: function() {
            NoteInput.chooseDefaultMode()
            NoteInput.chooseNoteDuration(Commands.SET_DURATION_QUARTER_COMMAND)
            for (var m = 0; m < NOTE_MEASURES; ++m) {
                for (var q = 0; q < QUARTERS.length; ++q) {
                    NoteInput.putNote(QUARTERS[q])
                }
            }
            // one 16th note in the next measure, for the custom ratio
            NoteInput.chooseNoteDuration(Commands.SET_DURATION_16TH_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            NoteInput.turnoffDefaultMode()
            api.testflow.seeChanges()
            Tuplets.expectRhythm(0, "4 4 4 4", "put notes")
            Tuplets.expectRhythm(SIXTEENTH_MEASURE, "16 r16 r8 r4 r2", "put notes")
        }},

        // A. Tuplet commands in normal mode
        {name: "A1 Triplet command on a quarter note, default option", func: function() {
            selectAt(0)
            Tuplets.send(ADD_TRIPLET_COMMAND)
            Tuplets.expectRhythm(0, "3:2[4 r4 r4] 4 4", "triplet, default option")
        }},
        {name: "A2 Triplet on a quarter note, Subdivision", func: function() {
            selectAt(1)
            Tuplets.send(addTupletCommand("ratio=3/2&selected-duration-as=subdivision"))
            Tuplets.expectRhythm(1, "3:2[4 r4 r4] 4 4", "triplet, Subdivision")
        }},
        {name: "A3 Triplet on a quarter note, Tuplet length", func: function() {
            selectAt(2)
            Tuplets.send(addTupletCommand("ratio=3/2&selected-duration-as=tuplet-length"))
            Tuplets.expectRhythm(2, "3:2[8 r8 r8] 4 4 4", "triplet, Tuplet length")
        }},
        {name: "A4 Duplet on a quarter note, Subdivision", func: function() {
            selectAt(3)
            Tuplets.send(addTupletCommand("ratio=2/2&selected-duration-as=subdivision"))
            Tuplets.expectRhythm(3, "2:3[4 r4] 4", "duplet, Subdivision")
        }},
        {name: "A5 Quintuplet command on a quarter note, default option", func: function() {
            selectAt(4)
            Tuplets.send(ADD_QUINTUPLET_COMMAND)
            Tuplets.expectRhythm(4, "5:4[4 r4 r4 r4 r4]", "quintuplet, default option")
        }},
        {name: "A6 Septuplet on a quarter note, Tuplet length", func: function() {
            selectAt(5)
            Tuplets.send(addTupletCommand("ratio=7/2&selected-duration-as=tuplet-length"))
            Tuplets.expectRhythm(5, "7:4[16 " + repeated("r16", 6) + "] 4 4 4", "septuplet, Tuplet length")
        }},
        {name: "A7 Nonuplet on a quarter note, Tuplet length", func: function() {
            selectAt(6)
            Tuplets.send(addTupletCommand("ratio=9/2&selected-duration-as=tuplet-length"))
            Tuplets.expectRhythm(6, "9:8[32 " + repeated("r32", 8) + "] 4 4 4", "nonuplet, Tuplet length")
        }},
        {name: "A8 Triplets on a range of two quarter notes, Tuplet length", func: function() {
            selectRange(7, 0, 2)
            Tuplets.send(addTupletCommand("ratio=3/2&selected-duration-as=tuplet-length"))
            Tuplets.expectRhythm(7, "3:2[8 r8 r8] 3:2[8 r8 r8] 4 4", "range, Tuplet length")
        }},
        {name: "A9 Triplet on a measure rest, Tuplet length", func: function() {
            selectAt(REST_MEASURE)
            Tuplets.send(addTupletCommand("ratio=3/2&selected-duration-as=tuplet-length"))
            Tuplets.expectRhythm(REST_MEASURE, "3:2[r2 r2 r2]", "measure rest, Tuplet length")
        }},

        // B. Other…: a custom ratio, as typed (auto-baselen=false), and the number and bracket types
        {name: "B1 Custom 23/2 on a 16th note, Subdivision", func: function() {
            selectAt(SIXTEENTH_MEASURE)
            Tuplets.send(addTupletCommand("ratio=23/2&auto-baselen=false&selected-duration-as=subdivision"))
            Tuplets.expectRhythm(SIXTEENTH_MEASURE, "23:2[16 " + repeated("r16", 22) + "] r8 r4 r2", "custom 23/2, Subdivision")
        }},
        {name: "B2 Custom 6/4 on a quarter note, Tuplet length", func: function() {
            selectAt(7, 2)
            Tuplets.send(addTupletCommand("ratio=6/4&auto-baselen=false&selected-duration-as=tuplet-length"))
            Tuplets.expectRhythm(7, "3:2[8 r8 r8] 3:2[8 r8 r8] 6:4[16 " + repeated("r16", 5) + "] 4", "custom 6/4, Tuplet length")
        }},
        {name: "B3 Number type and bracket type", func: function() {
            selectAt(7, 3)
            Tuplets.send(addTupletCommand("ratio=3/2&auto-baselen=false&selected-duration-as=tuplet-length"
                                          + "&number-type=relation&bracket-type=show"))
            var tuplet = Tuplets.tupletAt(7, 3 * QUARTER_TICKS)
            if (!tuplet || tuplet.numberType !== engraving.TupletNumberType.SHOW_RELATION
                || tuplet.bracketType !== engraving.TupletBracketType.SHOW_BRACKET) {
                api.testflow.fatal("number and bracket type: expected a 3:2 tuplet with ratio and bracket, got "
                                   + (tuplet ? tuplet.numberType + "/" + tuplet.bracketType : "no tuplet"))
            }
            api.log.info("[testflow] number and bracket type: OK")
        }},

        // C. Note input: the tuplet command in the input methods (a tuplet of the input duration at the input position)
        {name: "C1 Input by note name: eighth, triplet, three notes", func: function() {
            selectAt(NOTE_INPUT_MEASURE)
            setNoteInput("by-note-name", true)
            Tuplets.send(Commands.SET_DURATION_EIGHTH_COMMAND)
            Tuplets.send(ADD_TRIPLET_COMMAND)
            NoteInput.putNotes(Commands.ENTER_NOTE_C_COMMAND, 3)
            setNoteInput("by-note-name", false)
            Tuplets.expectRhythm(NOTE_INPUT_MEASURE, "3:2[8 8 8] r4 r2", "by note name")
        }},
        {name: "C2 Input by duration: eighth, triplet, three eighths", func: function() {
            selectAt(NOTE_INPUT_MEASURE + 1)
            setNoteInput("by-duration", true)
            Tuplets.send(Commands.SET_DURATION_EIGHTH_COMMAND)
            Tuplets.send(ADD_TRIPLET_COMMAND)
            for (var i = 0; i < 3; ++i) {
                Tuplets.send(Commands.SET_DURATION_EIGHTH_COMMAND)
            }
            setNoteInput("by-duration", false)
            Tuplets.expectRhythm(NOTE_INPUT_MEASURE + 1, "8 3:2[8 8 8] r8 r2", "by duration")
        }},
        {name: "C3 Rhythm only: eighth, triplet, three eighths", func: function() {
            selectAt(NOTE_INPUT_MEASURE + 2)
            setNoteInput("rhythm", true)
            Tuplets.send(Commands.SET_DURATION_EIGHTH_COMMAND)
            Tuplets.send(ADD_TRIPLET_COMMAND)
            for (var i = 0; i < 3; ++i) {
                Tuplets.send(Commands.SET_DURATION_EIGHTH_COMMAND)
            }
            setNoteInput("rhythm", false)
            Tuplets.expectRhythm(NOTE_INPUT_MEASURE + 2, "8 3:2[8 8 8] r8 r2", "rhythm only")
        }},
        {name: "C4 Nested: a triplet of eighths in a triplet of quarters, by note name", func: function() {
            selectAt(NOTE_INPUT_MEASURE + 3)
            setNoteInput("by-note-name", true)
            Tuplets.send(Commands.SET_DURATION_QUARTER_COMMAND)
            Tuplets.send(ADD_TRIPLET_COMMAND)
            Tuplets.send(Commands.SET_DURATION_EIGHTH_COMMAND)
            Tuplets.send(ADD_TRIPLET_COMMAND)
            NoteInput.putNotes(Commands.ENTER_NOTE_C_COMMAND, 3)
            Tuplets.send(Commands.SET_DURATION_QUARTER_COMMAND)
            NoteInput.putNotes(Commands.ENTER_NOTE_D_COMMAND, 2)
            setNoteInput("by-note-name", false)
            Tuplets.expectRhythm(NOTE_INPUT_MEASURE + 3, "3:2[ 3:2[8 8 8] 4 4] r2", "nested")
        }},

        // D. Edits of tuplets
        {name: "D1 Delete a tuplet: its notes selected as a range, then Delete", func: function() {
            // the API can't select a tuplet itself
            selectRange(0, 0, 2)
            Tuplets.send(DELETE_COMMAND)
            Tuplets.expectRhythm(0, "r2 4 4", "delete a tuplet")
        }},
        {name: "D2 Halve and double a range with a triplet", func: function() {
            selectRange(2, 0, 4)
            Tuplets.send(HALVE_DURATION_COMMAND)
            Tuplets.expectRhythm(2, "3:2[16 r16 r16] 8 8 8 r2", "halve a range")
            Tuplets.send(DOUBLE_DURATION_COMMAND)
            Tuplets.expectRhythm(2, "3:2[8 r8 r8] 4 4 4", "double it back")
        }},
        {name: "D3 Copy a measure with tuplets and paste it into an empty measure", func: function() {
            selectRange(7, 0, 4)
            Tuplets.send(COPY_COMMAND)
            selectAt(PASTE_MEASURE)
            Tuplets.send(PASTE_COMMAND)
            Tuplets.expectRhythm(PASTE_MEASURE, Tuplets.rhythm(7), "copy and paste")
        }},
        {name: "D4 Change a duration inside a triplet", func: function() {
            selectAt(1)
            Tuplets.send(Commands.SET_DURATION_HALF_COMMAND)
            Tuplets.expectRhythm(1, "3:2[2 r4] 4 4", "a half in a triplet of quarters")
        }},

        // E–G. Parts, undo/redo, save and reopen
        {name: "E Parts match their staves, tuplets included", func: function() {
            var parts = Parts.list()
            for (var i = 0; i < parts.length; ++i) {
                ScoreContent.expectSameLines(ScoreContent.contentLines(engraving.curScore, i),
                                             ScoreContent.contentLines(parts[i].score, 0), "part " + parts[i].title)
            }
            ScoreContent.expectSameLines(allRhythms(), allRhythms(parts[0].score), "flute part rhythms")
        }},
        {name: "F Undo all, then redo all", func: function() {
            finalState = snapshotAll()
            UndoRedo.fullCycle(initialState, ScoreState.snapshot())
            expectSameSnapshot(finalState, snapshotAll(), "after undo all and redo all")
        }},
        {name: "G Save, close and reopen", func: function() {
            savedState = snapshotAll()
            api.testflow.saveProject(FILE_NAME)
            api.testflow.seeChanges()
            api.dispatcher.dispatch(Commands.PROJECT_CLOSE_COMMAND)
            api.testflow.seeChanges()
            if (Project.isOpened()) {
                api.testflow.fatal("the score is still open after closing")
            }
            Home.goToHome()
            api.testflow.seeChanges()
            Home.openLastProject()
            if (!waitForProject(10000)) {
                api.testflow.fatal("the score did not reopen")
            }
            api.testflow.seeChanges()
            Parts.goToMainScore()
            expectSameSnapshot(savedState, snapshotAll(), "after reopen")
        }}
    ]
};

function main()
{
    api.testflow.setInterval(500)
    api.testflow.runTestCase(testCase)
}
