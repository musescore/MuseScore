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

function main()
{
    var testCase = {
        name: "Create Simple Score",
        description: "Just create a simple score by template, a few notes, play it and save the project",
        steps: [
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
                api.autobot.saveProject("TC1_CreateSimpleScore.mscz")
            }},
            {name: "Close", func: function() {
                api.dispatcher.dispatch("file-close")
            }},
            {name: "Home", func: function() {
                // Go Home
                api.navigation.triggerControl("TopTool", "MainToolBar", "Home")
            }},
            {name: "Open last", func: function() {
                api.navigation.goToControl("RecentScores", "RecentScores", "New score")
                api.navigation.right()
                api.navigation.trigger()
            }}
        ]
    };

    api.autobot.setInterval(1000)
    api.autobot.runTestCase(testCase)
}
