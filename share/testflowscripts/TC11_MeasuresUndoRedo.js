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
var ScoreState = require("steps/ScoreState.js")
var UndoRedo = require("steps/UndoRedo.js")
var Home = require("steps/Home.js")
var Project = require("steps/Project.js")
var Commands = require("steps/Commands.js")
var engraving = require("MuseApi.Engraving")

const APPEND_MEASURE_COMMAND = "command://notation/append-measure"
const INSERT_MEASURE_COMMAND = "command://notation/insert-measure"
const REMOVE_SELECTED_RANGE_COMMAND = "command://notation/remove-selected-range"

const NOTES = [
    Commands.ENTER_NOTE_C_COMMAND, Commands.ENTER_NOTE_D_COMMAND,
    Commands.ENTER_NOTE_E_COMMAND, Commands.ENTER_NOTE_F_COMMAND,
    Commands.ENTER_NOTE_G_COMMAND, Commands.ENTER_NOTE_A_COMMAND,
    Commands.ENTER_NOTE_B_COMMAND, Commands.ENTER_NOTE_C_COMMAND
]

//! NOTE States recorded during the test, checked again after undo/redo
var initialState = null
var finalState = null
var measures = 0

//! NOTE The notes as entered (track 0), and how long one measure is
var enteredNotes = []
var measureTicks = 0

//! NOTE How many undo steps the test has made so far; each note, appended,
//! inserted or deleted measure counts as one
var undoSteps = 0

//! NOTE Pitch class (pitch % 12) of each entered note: C D E F G A B C
const PITCH_CLASSES = [0, 2, 4, 5, 7, 9, 11, 0]
const QUARTER_TICKS = 480

function shifted(notes, ticks)
{
    return notes.map(function(n) { return { tick: n.tick + ticks, duration: n.duration, pitch: n.pitch } })
}

var testCase = {
    name: "TC11: Measures and Undo/Redo",
    description: "Enter notes, append, insert and delete measures (including measures with notes), check measure count and note positions after each step, then undo and redo everything, checking the number of steps and the score content",
    steps: [
        {name: "Close score (if opened) and go to home to start", func: function() {
            Project.closeIfOpened()
            Home.goToHome()
        }},
        {name: "Open New Score Dialog", func: function() {
            NewScore.openNewScoreDialog()
        }},
        {name: "Create score with Flute", func: function() {
            NewScore.chooseInstrument("Woodwinds", "Flute")
            NewScore.done()
        }},
        {name: "Remember the initial state", func: function() {
            initialState = ScoreState.snapshot()
            if (!initialState) {
                api.testflow.fatal("no score is opened")
            }
            measures = initialState.measures
            measureTicks = engraving.curScore.firstMeasure.ticks.ticks
            api.log.info("[testflow] initial: " + ScoreState.describe(initialState))
        }},
        {name: "Put notes", func: function() {
            NoteInput.chooseDefaultMode()
            NoteInput.chooseNoteDuration(Commands.SET_DURATION_QUARTER_COMMAND)
            for (var i = 0; i < NOTES.length; ++i) {
                NoteInput.putNote(NOTES[i])
            }
            NoteInput.turnoffDefaultMode()
            api.testflow.seeChanges()
            undoSteps += NOTES.length
            ScoreState.expectNotes(NOTES.length, "put notes")
            ScoreState.expectMeasures(measures, "put notes")

            // Check each note: one quarter note after another from the start, with the right pitch
            enteredNotes = ScoreState.notesOnTrack(0)
            for (var j = 0; j < enteredNotes.length; ++j) {
                var n = enteredNotes[j]
                if (n.tick !== j * QUARTER_TICKS || n.duration !== QUARTER_TICKS || n.pitch % 12 !== PITCH_CLASSES[j]) {
                    api.testflow.fatal("put notes: note " + (j + 1) + " is wrong (tick " + n.tick + ", duration " + n.duration + ", pitch " + n.pitch + ")")
                }
            }
            api.log.info("[testflow] put notes: OK (positions, durations and pitches)")
        }},
        {name: "Append 4 measures", func: function() {
            for (var i = 0; i < 4; ++i) {
                api.dispatcher.dispatch(APPEND_MEASURE_COMMAND)
            }
            api.testflow.seeChanges()
            measures += 4
            undoSteps += 4
            ScoreState.expectMeasures(measures, "append 4 measures")
            ScoreState.expectTrackNotes(0, enteredNotes, "append 4 measures")
        }},
        {name: "Insert a measure before the first note", func: function() {
            Score.firstElement()
            if (!Score.findNote("Alt+Right", undefined, 20)) {
                api.testflow.fatal("the first note is not found")
            }
            api.dispatcher.dispatch(INSERT_MEASURE_COMMAND)
            api.testflow.seeChanges()
            measures += 1
            undoSteps += 1
            ScoreState.expectMeasures(measures, "insert measure")
            // all notes move one measure later
            ScoreState.expectTrackNotes(0, shifted(enteredNotes, measureTicks), "insert measure")
        }},
        {name: "Delete a measure with notes and the empty measure after it", func: function() {
            // After the insert: measure 1 is empty, measure 2 has C D E F,
            // measure 3 has G A B C, measure 4 is empty. Delete measures 3 and 4.
            ScoreState.selectMeasures(2, 2)
            api.testflow.seeChanges()
            api.dispatcher.dispatch(REMOVE_SELECTED_RANGE_COMMAND)
            api.testflow.seeChanges()
            measures -= 2
            undoSteps += 1
            ScoreState.expectMeasures(measures, "delete 2 measures")
            // only C D E F are left, still in measure 2
            ScoreState.expectTrackNotes(0, shifted(enteredNotes.slice(0, 4), measureTicks), "delete 2 measures")
        }},
        {name: "Remember the final state", func: function() {
            finalState = ScoreState.snapshot()
            api.log.info("[testflow] final: " + ScoreState.describe(finalState) + ", undo steps: " + undoSteps)
        }},
        {name: "Undo all, then redo all", func: function() {
            UndoRedo.fullCycle(initialState, finalState, undoSteps)
        }},
        {name: "Save", func: function() {
            api.testflow.saveProject("TC11_MeasuresUndoRedo.mscz")
        }}
    ]
};

function main()
{
    api.testflow.setInterval(500)
    api.testflow.runTestCase(testCase)
}
