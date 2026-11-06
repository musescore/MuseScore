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
var Instruments = require("steps/Instruments.js")
var Project = require("steps/Project.js")
var Commands = require("steps/Commands.js")

const PITCH_E5 = 76

//! NOTE The text edit starts asynchronously, the typed keys must come after it is started
function waitTextEdit()
{
    api.testflow.sleep(300)
}

function startTextEdit(shortcut)
{
    api.shortcuts.activate(shortcut)
    waitTextEdit()
}

var testCase = {
    name: "TC8: Engraving Text",
    description: "Let's check the functionality of the edit engraving text",
    steps: [
        {name: "Close score (if opened) and go to home to start", func: function() {
            Project.closeIfOpened()
            Navigation.triggerControl("TopTool", "MainToolBar", "Home")
        }},
        {name: "Open New Score Dialog", func: function() {
            NewScore.openNewScoreDialog()
        }},
        {name: "Create score with Flute", func: function() {
            NewScore.selectTab("instruments")
            NewScore.chooseInstrument("Woodwinds", "Flute")
            NewScore.done()
        }},
        {name: "Put notes", func: function() {
            NoteInput.chooseDefaultMode()
            NoteInput.putNote(Commands.ENTER_NOTE_A_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_B_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_D_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_E_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_F_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_E_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_D_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_B_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_A_COMMAND)
            NoteInput.turnoffDefaultMode()
        }},
        {name: "Add Lyrics", skip: false, func: function() {
            Score.focusIn()
            for (var i = 0; i < 11; ++i) {
                Score.prevChord()
            }
            seeChanges()

            startTextEdit("Ctrl+L")
            api.keyboard.text("la")
            api.keyboard.key(" ")
            seeChangesFast()
            api.keyboard.text("la")
            api.keyboard.key(" ")
            seeChangesFast()
            api.keyboard.text("la")
            api.keyboard.key(" ")
            seeChangesFast()
            api.keyboard.text("tra")
            api.keyboard.key(" ")
            seeChangesFast()
            api.keyboard.text("la")
            api.keyboard.key(" ")
            seeChangesFast()
            api.keyboard.text("la")
            api.keyboard.key(" ")
            seeChangesFast()
            api.keyboard.text("tra")
            api.keyboard.key(" ")
            seeChangesFast()
            api.keyboard.text("la")
            api.keyboard.key(" ")
            seeChangesFast()
            api.keyboard.text("la")
            api.keyboard.key(" ")
            seeChangesFast()
            api.keyboard.text("bum")
            api.keyboard.key(" ")
            seeChangesFast()
            api.keyboard.text("bim")

            api.shortcuts.activate("Esc")
        }},
        {name: "Remove (change) Lyrics", skip: false, func: function() {
            //! NOTE Delete the selected lyric (outside of text editing it is a shortcut)
            api.shortcuts.activate("Backspace")
            seeChanges()

            startTextEdit("Ctrl+L")
            api.keyboard.text("bum")

            api.shortcuts.activate("Esc")
        }},
        {name: "Add StaffText", skip: false, func: function() {

            //! NOTE Ending the text edit clears the selection, so start from the score beginning
            Score.firstElement()

            //! NOTE Find note E5
            if (!Score.findNote("Alt+Right", PITCH_E5, 99)) {
                api.testflow.fatal("Note E5 is not found")
            }

            //! NOTE Add StaffText
            startTextEdit("Ctrl+T")
            api.keyboard.text("This is Sparta")
            api.shortcuts.activate("Esc") // end edit
        }},
        {name: "Edit StaffText", skip: false, func: function() {

            api.keyboard.key("Return") // begin edit
            waitTextEdit()
            seeChanges()
            api.keyboard.key("End") // move cursor to end
            seeChanges()
            api.keyboard.repeatKey("Backspace", 6) // remove Sparta
            seeChanges()
            api.keyboard.text("Flute") // add Flute
            seeChanges()
            api.shortcuts.activate("Esc") // end edit
        }},
        {name: "Add Oboe Instrument", func: function() {
            Instruments.openInstrumentsPanel()
            Instruments.openAddInstrumentsDialog()
        }},
        {name: "Select Oboe Instrument", func: function() {
            Instruments.chooseInstrument("Woodwinds", "Oboe")
            Instruments.doneAddInstruments()
        }},
        {name: "Add StaffText to Oboe", func: function() {
            //! NOTE Adding an instrument resets the selection, so start from the score beginning
            Score.firstElement()

            //! NOTE Find Note
            if (!Score.findNote("Alt+Right", undefined, 9)) {
                api.testflow.fatal("Current item should be Note")
            }

            api.shortcuts.activate("Alt+Down")
            seeChanges()

            startTextEdit("Ctrl+T")
            api.keyboard.text("This is Oboe")
            api.shortcuts.activate("Esc")
        }},
        {name: "Add System Text", func: function() {
            Score.focusIn()
            api.shortcuts.activate("Ctrl+Home")
            //! NOTE Find first Note
            if (!Score.findNote("Alt+Right", undefined, 99)) {
                api.testflow.fatal("Note is not found")
            }
            for (var m = 0; m < 6; ++m) {
                api.shortcuts.activate("Ctrl+Right")
                seeChangesFast()
            }

            //! NOTE Add System Text
            startTextEdit("Ctrl+Shift+T")
            api.keyboard.text("This is Flute and Oboe")
            api.shortcuts.activate("Esc")
        }},
        {name: "Hide/Show Flute", func: function() {
            Instruments.openInstrumentsPanel()
            Instruments.toggleVisibleInstrument("Flute")
            seeChangesLong()
            seeChangesLong()
            Instruments.toggleVisibleInstrument("Flute")
        }},
        {name: "Hide/Show Oboe", func: function() {
            Instruments.openInstrumentsPanel()
            Instruments.toggleVisibleInstrument("Oboe")
            seeChangesLong()
            seeChangesLong()
            Instruments.toggleVisibleInstrument("Oboe")
        }},
        {name: "Save", func: function() {
            api.testflow.saveProject("TC8_EngravingText.mscz")
        }},
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

function seeChangesLong()
{
    api.testflow.seeChanges(1000)
}

function seeChangesFast()
{
    api.testflow.seeChanges(200)
}

