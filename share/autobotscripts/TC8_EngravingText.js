/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
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

var testCase = {
    name: "TC8: Engraving Text",
    description: "Let's check the functionality of the edit engraving text",
    steps: [
        {name: "Close score (if opened) and go to home to start", func: function() {
            api.dispatcher.dispatch("file-close")
            api.navigation.triggerControl("TopTool", "MainToolBar", "Home")
        }},
        {name: "Open New Score Dialog", func: function() {
            NewScore.openNewScoreDialog()
        }},
        {name: "Create score with Flute", func: function() {
            NewScore.selectTab("instruments")
            NewScore.chooseInstrument("Free Reed", "Accordion")
            NewScore.done()
        }},
        {name: "Put notes", func: function() {
            api.shortcuts.activate("A")
            api.shortcuts.activate("B")
            api.shortcuts.activate("C")
            api.shortcuts.activate("D")
            api.shortcuts.activate("E")
            api.shortcuts.activate("F")
            api.shortcuts.activate("E")
            api.shortcuts.activate("D")
            api.shortcuts.activate("C")
            api.shortcuts.activate("B")
            api.shortcuts.activate("A")
        }},
        {name: "Add Lyrics", skip: false, func: function() {
            Score.focusIn()
            for (var i = 0; i < 11; ++i) {
                Score.prevChord()
            }
            seeChanges()

            api.shortcuts.activate("Ctrl+L")
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

            api.keyboard.key("Esc")
        }},
        {name: "Remove (change) Lyrics", skip: false, func: function() {
            api.keyboard.key("Backspace")
            seeChanges()

            api.shortcuts.activate("Ctrl+L")
            api.keyboard.text("bum")

            api.keyboard.key("Esc")
        }},
        {name: "Add StaffText", skip: false, func: function() {

            //! NOTE Find note E
            for (var i = 0; i < 99; ++i) {
                api.shortcuts.activate("Alt+Left")
                seeChangesFast()
                var name = api.accessibility.currentName()
                if (name.startsWith("Note; Pitch: E5")) {
                    break;
                }
            }

            //! NOTE Add StaffText
            api.shortcuts.activate("Ctrl+T")
            api.keyboard.text("This is Sparta")
            api.keyboard.key("Esc") // end edit
        }},
        {name: "Edit StaffText", skip: false, func: function() {

            api.keyboard.key("Return") // begin edit
            seeChanges()
            api.keyboard.key("End") // move cursor to end
            seeChanges()
            api.keyboard.repeatKey("Backspace", 6) // remove Sparta
            seeChanges()
            api.keyboard.text("Flute") // add Flute
            seeChanges()
            api.keyboard.key("Esc") // end edit
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
            Score.focusIn()

            //! NOTE Find Note
            for (var i = 0; i < 9; ++i) {
                var name = api.accessibility.currentName()
                if (name.startsWith("Note")) {
                    break;
                }
                api.shortcuts.activate("Alt+Left")
            }

            item = api.accessibility.currentName()
            if (!item.startsWith("Note")) {
                api.autobot.fatal("Current item should be Note")
            }

            api.shortcuts.activate("Alt+Down")
            seeChanges()

            api.shortcuts.activate("Ctrl+T")
            api.keyboard.text("This is Oboe")
            api.keyboard.key("Esc")
        }},
        {name: "Add System Text", func: function() {
            Score.focusIn()
            api.shortcuts.activate("Ctrl+Home")
            //! NOTE Find first Note
            for (var i = 0; i < 99; ++i) {
                api.shortcuts.activate("Alt+Right")
                seeChangesFast()
                var name = api.accessibility.currentName()
                if (name.startsWith("Note")) {
                    break;
                }
            }
            for (var m = 0; m < 6; ++m) {
                api.shortcuts.activate("Ctrl+Right")
                seeChangesFast()
            }

            //! NOTE Add System Text
            api.shortcuts.activate("Ctrl+Shift+T")
            api.keyboard.text("This is Flute and Oboe")
            api.keyboard.key("Esc")
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
            api.autobot.saveProject("TC8_EngravingText.mscz")
        }},
    ]
};

function main()
{
    api.autobot.setInterval(1000)
    api.autobot.runTestCase(testCase)
}

function seeChanges()
{
    api.autobot.seeChanges()
}

function seeChangesLong()
{
    api.autobot.seeChanges(1000)
}

function seeChangesFast()
{
    api.autobot.seeChanges(200)
}

