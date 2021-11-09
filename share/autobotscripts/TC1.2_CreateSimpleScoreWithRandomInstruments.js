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

var testCase = {
    name: "TC1.2: Create Simple Score with Random Instruments",
    description: "Just create a simple two-instrument score, a few notes, play it and save the project",
    steps: [
        {name: "Open Dialog", func: function() {
            NewScore.openNewScoreDialog()
        }},
        {name: "Select Instruments", func: function() {
            NewScore.chooseRandomInstruments(10)
        }},
        {name: "Note input mode", func: function() {

            api.navigation.triggerControl("NoteInputSection", "NoteInputBar", "note-input-steptime")
            api.autobot.waitPopup()
            // First item become automatically current, so just trigger
            api.navigation.trigger()

            // Select note
            api.navigation.triggerControl("NoteInputSection", "NoteInputBar", "pad-note-8")
        }},
        {name: "Note input", func: function() {
            api.dispatcher.dispatch("note-c")
            api.dispatcher.dispatch("note-d")
            api.dispatcher.dispatch("note-e")
            api.dispatcher.dispatch("note-f")
            api.dispatcher.dispatch("note-g")
            api.dispatcher.dispatch("note-a")
            api.dispatcher.dispatch("note-b")
        }},
        {name: "Save", func: function() {
            api.autobot.saveProject("TC1.2_CreateSimpleScoreWithRandomInstruments.mscz")
        }},
        {name: "Close", func: function() {
            api.dispatcher.dispatch("file-close")
            api.autobot.seeChanges()
        }},
        {name: "Home", func: function() {
            api.navigation.triggerControl("TopTool", "MainToolBar", "Home")
        }},
        {name: "Open last", func: function() {
            api.navigation.goToControl("RecentScores", "RecentScores", "New score")
            api.navigation.right()
            api.navigation.trigger()
        }}
    ]
};

function main()
{
    api.autobot.setInterval(1000)
    api.autobot.runTestCase(testCase)
}
