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
    name: "TC1.2: Create Simple Score with Random Instruments",
    description: "Just create a simple two-instrument score, a few notes, play it and save the project",
    steps: [
        {name: "Close score (if opened) and go to home to start", func: function() {
            Project.closeIfOpened()
            Home.goToHome()
        }},
        {name: "Open New Score Dialog", func: function() {
            NewScore.openNewScoreDialog()
        }},
        {name: "Select Instruments", func: function() {
            NewScore.selectTab("instruments")
            NewScore.chooseRandomInstruments(10)
        }},
        {name: "Create score", func: function() {
            NewScore.done()
        }},
        {name: "Note input mode", func: function() {
            NoteInput.chooseDefaultMode()
            NoteInput.chooseNoteDuration(Commands.SET_DURATION_EIGHTH_COMMAND)
        }},
        {name: "Note input", func: function() {
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_D_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_E_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_F_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_G_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_A_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_B_COMMAND)
        }},
        {name: "Save", func: function() {
            api.testflow.saveProject("TC1.2_CreateSimpleScoreWithRandomInstruments.mscz")
        }},
        {name: "Close", func: function() {
            api.dispatcher.dispatch(Commands.PROJECT_CLOSE_COMMAND)
            api.testflow.seeChanges()
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
