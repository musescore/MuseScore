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
var Home = require("steps/Home.js")

var testCase = {
    name: "TC2: Create Score by Template",
    description: "Just create a simple score by template, a few notes, play it and save the project",
    steps: [
        {name: "Close score (if opened) and go to home to start", func: function() {
            api.dispatcher.dispatch("file-close")
            Home.goToHome()
        }},
        {name: "Open New Score Dialog", func: function() {
            NewScore.openNewScoreDialog()
        }},
        {name: "Choose template", func: function() {
            NewScore.selectTab("templates")
            NewScore.chooseTemplate("General", "Grand Staff")
        }},
        {name: "Create score", func: function() {
            NewScore.done()
        }},
        {name: "Turn on note input", func: function() {
            NoteInput.chooseDefaultMode()
            NoteInput.chooseNoteDuration("pad-note-8")
        }},
        {name: "Put notes", func: function() {
            NoteInput.putNote("note-c")
            NoteInput.putNote("note-d")
            NoteInput.putNote("note-e")
            NoteInput.putNote("note-f")
            NoteInput.putNote("note-g")
            NoteInput.putNote("note-a")
            NoteInput.putNote("note-b")
        }},
        {name: "Play", func: function() {
            api.navigation.triggerControl("TopTool", "PlaybackToolBar", "Play")
        }},
        {name: "Stop", func: function() {
            // wait interval + 5 sec
            api.autobot.sleep(5000)
            api.navigation.triggerControl("TopTool", "PlaybackToolBar", "Play")
        }},
        {name: "Save", func: function() {
            api.autobot.saveProject("TC2_CreateSimpleScoreByTemplate.mscz")
        }},
        {name: "Close", func: function() {
            api.dispatcher.dispatch("file-close")
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
    api.autobot.setInterval(1000)
    api.autobot.runTestCase(testCase)
}
