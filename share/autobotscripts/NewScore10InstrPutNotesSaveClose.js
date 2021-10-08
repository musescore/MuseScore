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

function main()
{
    var testCase = {
        name: "NewScore10InstrPutNotesSaveClose",
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
                api.autobot.saveProject("NewScore10InstrPutNotesSaveClose.mscz")
            }},
            {name: "Close", func: function() {
                api.dispatcher.dispatch("file-close")
                api.autobot.seeChanges()

                // Go Home
                api.navigation.triggerControl("TopTool", "MainToolBar", "Home")
            }}
        ]
    };

    api.autobot.setInterval(1000)
    api.autobot.runTestCase(testCase)
    api.log.info("----------- end script ---------------")
}
