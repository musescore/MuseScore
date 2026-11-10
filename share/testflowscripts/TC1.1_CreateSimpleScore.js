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
var Navigation = require("steps/Navigation.js")
var Score = require("steps/Score.js")
var Home = require("steps/Home.js")
var Project = require("steps/Project.js")
var Commands = require("steps/Commands.js")

var testCase = {
    name: "TC1.1: Create Simple Score",
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
            NewScore.chooseInstrument("Free Reed", "Accordion")
            api.testflow.seeChanges()
            NewScore.chooseInstrument("Keyboards", "Piano")
        }},
        {name: "Create score", func: function() {
            NewScore.done()
        }},
        {name: "Turn on note input", func: function() {
            NoteInput.chooseDefaultMode()
            NoteInput.chooseNoteDuration(Commands.SET_DURATION_EIGHTH_COMMAND)
        }},
        {name: "Put notes", func: function() {
            for (let i = 0; i < 3; ++i) {
                NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
                api.testflow.seeChanges()
                NoteInput.putNote(Commands.ENTER_NOTE_D_COMMAND)
                api.testflow.seeChanges()
                NoteInput.putNote(Commands.ENTER_NOTE_E_COMMAND)
                api.testflow.seeChanges()
                NoteInput.putNote(Commands.ENTER_NOTE_F_COMMAND)
                api.testflow.seeChanges()
                NoteInput.putNote(Commands.ENTER_NOTE_G_COMMAND)
                api.testflow.seeChanges()
                NoteInput.putNote(Commands.ENTER_NOTE_A_COMMAND)
                api.testflow.seeChanges()
                NoteInput.putNote(Commands.ENTER_NOTE_B_COMMAND)
                api.testflow.seeChanges()
            }
        }},
        {name: "Play", func: function() {
            //Navigation.triggerControl("TopTool", "PlaybackToolBar", "Play")
            Score.firstElement()
            api.dispatcher.dispatch(Commands.PLAY_COMMAND)
        }},
        {name: "Stop", func: function() {
            // wait interval + 5 sec
            api.testflow.sleep(5000)
            //Navigation.triggerControl("TopTool", "PlaybackToolBar", "Play")
            api.dispatcher.dispatch(Commands.PAUSE_COMMAND)
        }},
        {name: "Save", func: function() {
            api.testflow.saveProject("TC1.1_CreateSimpleScore.mscz")
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
