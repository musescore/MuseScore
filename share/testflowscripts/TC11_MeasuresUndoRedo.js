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

var testCase = {
    name: "TC11: Measures and Undo/Redo",
    description: "Append, insert and delete measures, check the measure count after each step, then undo and redo everything and check the score",
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
            ScoreState.expectNotes(NOTES.length, "put notes")
            ScoreState.expectMeasures(measures, "put notes")
        }},
        {name: "Append 4 measures", func: function() {
            for (var i = 0; i < 4; ++i) {
                api.dispatcher.dispatch(APPEND_MEASURE_COMMAND)
            }
            api.testflow.seeChanges()
            measures += 4
            ScoreState.expectMeasures(measures, "append 4 measures")
        }},
        {name: "Insert a measure before the first note", func: function() {
            Score.firstElement()
            if (!Score.findNote("Alt+Right", undefined, 20)) {
                api.testflow.fatal("the first note is not found")
            }
            api.dispatcher.dispatch(INSERT_MEASURE_COMMAND)
            api.testflow.seeChanges()
            measures += 1
            ScoreState.expectMeasures(measures, "insert measure")
            ScoreState.expectNotes(NOTES.length, "insert measure")
        }},
        {name: "Delete 2 measures near the end", func: function() {
            // the two measures just before the last one
            ScoreState.selectMeasures(measures - 3, 2)
            api.testflow.seeChanges()
            api.dispatcher.dispatch(REMOVE_SELECTED_RANGE_COMMAND)
            api.testflow.seeChanges()
            measures -= 2
            ScoreState.expectMeasures(measures, "delete 2 measures")
            ScoreState.expectNotes(NOTES.length, "delete 2 measures")
        }},
        {name: "Remember the final state", func: function() {
            finalState = ScoreState.snapshot()
            api.log.info("[testflow] final: " + ScoreState.describe(finalState))
        }},
        {name: "Undo all, then redo all", func: function() {
            UndoRedo.fullCycle(initialState, finalState)
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
