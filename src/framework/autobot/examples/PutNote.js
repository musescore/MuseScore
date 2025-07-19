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
        name: "New Score and Put Note",
        steps: [
            {name: "Open Dialog", func: function() {
//                api.dispatcher.dispatch("file-new")

//                OR

//                api.navigation.triggerControl("AppTitleBar", "AppMenuBar", "&File")
//                // wait popup open
//                api.autobot.waitPopup()
//                // New become automatically current, so just trigger
//                api.navigation.trigger()

//                OR
                  // use New Score step module
                  NewScore.openNewScoreDialog()

            }},
            {name: "Select Instruments", func: function() {

//                // Flute
//                api.navigation.goToControl("NewScoreDialog", "FamilyView", "Woodwinds")
//                api.navigation.goToControl("NewScoreDialog", "InstrumentsView", "Flute")
//                api.navigation.triggerControl("NewScoreDialog", "SelectPanel", "Select")

//                // just for see changes
//                api.autobot.sleep(500)

//                // Piano
//                api.navigation.goToControl("NewScoreDialog", "FamilyView", "Keyboards")
//                api.navigation.goToControl("NewScoreDialog", "InstrumentsView", "Piano")
//                api.navigation.triggerControl("NewScoreDialog", "SelectPanel", "Select")

//                // just for see changes
//                api.autobot.sleep(500)

//                // Done
//                api.navigation.triggerControl("NewScoreDialog", "BottomPanel", "Done")

                // OR
                // use New Score step module
                NewScore.chooseFluteAndPiano();

            }},
            {name: "Note input mode", func: function() {
                // api.dispatcher.dispatch("note-input")
                // api.dispatcher.dispatch("pad-note-8")

                // OR

                api.navigation.triggerControl("NoteInputSection", "NoteInputBar", "note-input-by-note-name")
                // wait popup open
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

                // OR

                // Open menu "+" and so on
            }},
            {name: "Undo", func: function() {

                // To perform undo with a shortcut, the UI context must be "NotationFocused", so go to notation for focused it
                api.navigation.goToControl("NotationView", "NotationViewTabs", "NotationTab0")

                api.shortcuts.activate("Ctrl+Z")
            }},
            {name: "Play", func: function() {
                // api.dispatcher.dispatch("play")

                // OR

                api.navigation.triggerControl("TopTool", "PlaybackToolBar", "Play")
            }},
        ]
    };

    api.autobot.setInterval(1000)
    api.autobot.runTestCase(testCase)
    api.log.info("----------- end script ---------------")
}
