/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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
            NewScore.сhooseInstrument("Woodwinds", "Flute")
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
        {name: "Add Lyrics", func: function() {
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

            api.keyboard.key("esc")
        }},
        {name: "Remove (change) Lyrics", func: function() {
            api.keyboard.key("backspace")
            seeChanges()

            api.shortcuts.activate("Ctrl+L")
            api.keyboard.text("bum")

            api.keyboard.key("esc")
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
    api.autobot.seeChanges(250)
}

