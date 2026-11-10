/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited and others
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
var Home = require("steps/Home.js")
var Project = require("steps/Project.js")
var Commands = require("steps/Commands.js")

var testCase = {
    name: "TC3: Using hote input toolbar",
    description: "Let's check the functionality of the buttons on the note input toolbar",
    steps: [
        {name: "Close score (if opened) and go to home to start", func: function() {
            Project.closeIfOpened()
            Home.goToHome()
        }},
        {name: "Open New Score Dialog", func: function() {
            NewScore.openNewScoreDialog()
        }},
        {name: "Create score", func: function() {
            NewScore.chooseInstrument("Woodwinds", "Flute")
            NewScore.done()
        }},
        {name: "Turn on note input", func: function() {
            NoteInput.chooseDefaultMode()
        }},
        {name: "Put", func: function() {
            NoteInput.chooseNoteDuration(Commands.SET_DURATION_64TH_COMMAND)
            NoteInput.putNotes(Commands.ENTER_NOTE_C_COMMAND, 16)
            api.testflow.seeChanges()
            NoteInput.chooseNoteDuration(Commands.SET_DURATION_32ND_COMMAND)
            NoteInput.putNotes(Commands.ENTER_NOTE_C_COMMAND, 8)
            api.testflow.seeChanges()
            NoteInput.chooseNoteDuration(Commands.SET_DURATION_16TH_COMMAND)
            NoteInput.putNotes(Commands.ENTER_NOTE_C_COMMAND, 4)
            api.testflow.seeChanges()
            NoteInput.chooseNoteDuration(Commands.SET_DURATION_EIGHTH_COMMAND)
            NoteInput.putNotes(Commands.ENTER_NOTE_C_COMMAND, 2)
            api.testflow.seeChanges()
            NoteInput.chooseNoteDuration(Commands.SET_DURATION_QUARTER_COMMAND)
            NoteInput.putNotes(Commands.ENTER_NOTE_C_COMMAND, 2)
            api.testflow.seeChanges()
            NoteInput.chooseNoteDuration(Commands.SET_DURATION_HALF_COMMAND)
            NoteInput.putNotes(Commands.ENTER_NOTE_C_COMMAND, 1)
            api.testflow.seeChanges()
            NoteInput.chooseNoteDuration(Commands.SET_DURATION_WHOLE_COMMAND)
            NoteInput.putNotes(Commands.ENTER_NOTE_C_COMMAND, 1)
            api.testflow.seeChanges()

            // note with dot
            NoteInput.chooseNoteDuration(Commands.SET_DURATION_QUARTER_COMMAND)
            NoteInput.toggleDot(Commands.TOGGLE_DOT_COMMAND)
            NoteInput.putNotes(Commands.ENTER_NOTE_C_COMMAND, 2)
            NoteInput.toggleDot(Commands.TOGGLE_DOT_COMMAND)
            NoteInput.putNotes(Commands.ENTER_NOTE_C_COMMAND, 1)
            api.testflow.seeChanges()

            // note with accidental
            NoteInput.chooseNoteDuration(Commands.SET_DURATION_QUARTER_COMMAND)
            NoteInput.toggleAccidental(Commands.TOGGLE_FLAT2_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            NoteInput.toggleAccidental(Commands.TOGGLE_FLAT_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            NoteInput.toggleAccidental(Commands.TOGGLE_NAT_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            NoteInput.toggleAccidental(Commands.TOGGLE_SHARP_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            NoteInput.chooseNoteDuration(Commands.SET_DURATION_WHOLE_COMMAND)
            NoteInput.toggleAccidental(Commands.TOGGLE_SHARP2_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)

            // tie
            NoteInput.chooseNoteDuration(Commands.SET_DURATION_QUARTER_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            NoteInput.putTie()
            NoteInput.putTie()
            NoteInput.putTie()

            // slur
            //! TODO The problem with the finish of the slur, see issues/7985
//                NoteInput.chooseNoteDuration(Commands.SET_DURATION_QUARTER_COMMAND)
//                NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
//                NoteInput.toggleSlur()
//                NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
//                NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
//                NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
//                NoteInput.toggleSlur()

            // articulation
            NoteInput.chooseNoteDuration(Commands.SET_DURATION_EIGHTH_COMMAND)
            NoteInput.toggleArticulation(Commands.TOGGLE_MARCATO_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            NoteInput.toggleArticulation(Commands.TOGGLE_SFORZATO_COMMAND) // `add-marcato` to `add-sforzato`
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            NoteInput.toggleArticulation(Commands.TOGGLE_SFORZATO_COMMAND) // `add-sforzato` off
            NoteInput.toggleArticulation(Commands.TOGGLE_TENUTO_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            NoteInput.toggleArticulation(Commands.TOGGLE_TENUTO_COMMAND) // `add-tenuto` off
            NoteInput.toggleArticulation(Commands.TOGGLE_STACCATO_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            NoteInput.toggleArticulation(Commands.TOGGLE_MARCATO_COMMAND) // `add-staccato` + `add-marcato`
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            NoteInput.toggleArticulation(Commands.TOGGLE_SFORZATO_COMMAND) // `add-staccato` + `add-sforzato`
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            NoteInput.toggleArticulation(Commands.TOGGLE_TENUTO_COMMAND) // `add-staccato` + `add-tenuto + `add-sforzato`
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            NoteInput.toggleArticulation(Commands.TOGGLE_MARCATO_COMMAND) // `add-staccato` + `add-tenuto + `add-marcato`
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            NoteInput.toggleArticulation(Commands.TOGGLE_MARCATO_COMMAND) // `add-marcato` off
            NoteInput.toggleArticulation(Commands.TOGGLE_TENUTO_COMMAND) // `add-tenuto` off
            NoteInput.toggleArticulation(Commands.TOGGLE_STACCATO_COMMAND) // `add-staccato` off

            NoteInput.chooseNoteDuration(Commands.SET_DURATION_QUARTER_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)

            // tuplets
            NoteInput.putTuplet(3)
            //! TODO Not completed
        }},
        {name: "Save", func: function() {
            api.testflow.saveProject("TC3 Using note input toolbar.mscz")
        }},
        {name: "Close", func: function() {
            api.dispatcher.dispatch(Commands.PROJECT_CLOSE_COMMAND)
        }},
        {name: "Home", func: function() {
            Home.goToHome()
        }},
        {name: "Open last", func: function() {
            Home.openLastProject()
        }}
    ]
};

function main()
{
    api.testflow.setInterval(1000)
    api.testflow.runTestCase(testCase)
}

function seeChanges()
{
    api.testflow.seeChanges()
}
