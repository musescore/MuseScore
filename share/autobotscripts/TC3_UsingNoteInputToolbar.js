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

var testCase = {
    name: "TC3: Using hote input toolbar",
    description: "Let's check the functionality of the buttons on the note input toolbar",
    steps: [
        {name: "Close score (if opened) and go to home to start", func: function() {
            api.dispatcher.dispatch("file-close")
            api.navigation.triggerControl("TopTool", "MainToolBar", "Home")
        }},
        {name: "Open New Score Dialog", func: function() {
            NewScore.openNewScoreDialog()
        }},
        {name: "Create score", func: function() {
            NewScore.chooseInstrument("Woodwinds", "Flute")
            NewScore.done()
        }},
        {name: "Turn on note input", func: function() {
            NoteInput.chooseDefaultMode()
        }},
        {name: "Put", func: function() {
            NoteInput.chooseNoteDuration("pad-note-64")
            NoteInput.putNotes("note-c", 16)
            api.autobot.seeChanges()
            NoteInput.chooseNoteDuration("pad-note-32")
            NoteInput.putNotes("note-c", 8)
            api.autobot.seeChanges()
            NoteInput.chooseNoteDuration("pad-note-16")
            NoteInput.putNotes("note-c", 4)
            api.autobot.seeChanges()
            NoteInput.chooseNoteDuration("pad-note-8")
            NoteInput.putNotes("note-c", 2)
            api.autobot.seeChanges()
            NoteInput.chooseNoteDuration("pad-note-4")
            NoteInput.putNotes("note-c", 2)
            api.autobot.seeChanges()
            NoteInput.chooseNoteDuration("pad-note-2")
            NoteInput.putNotes("note-c", 1)
            api.autobot.seeChanges()
            NoteInput.chooseNoteDuration("pad-note-1")
            NoteInput.putNotes("note-c", 1)
            api.autobot.seeChanges()

            // note with dot
            NoteInput.chooseNoteDuration("pad-note-4")
            NoteInput.toggleDot("pad-dot")
            NoteInput.putNotes("note-c", 2)
            NoteInput.toggleDot("pad-dot")
            NoteInput.putNotes("note-c", 1)
            api.autobot.seeChanges()

            // note with accidental
            NoteInput.chooseNoteDuration("pad-note-4")
            NoteInput.toggleAccidental("flat2")
            NoteInput.putNote("note-c")
            NoteInput.toggleAccidental("flat")
            NoteInput.putNote("note-c")
            NoteInput.toggleAccidental("nat")
            NoteInput.putNote("note-c")
            NoteInput.toggleAccidental("sharp")
            NoteInput.putNote("note-c")
            NoteInput.chooseNoteDuration("pad-note-1")
            NoteInput.toggleAccidental("sharp2")
            NoteInput.putNote("note-c")

            // tie
            NoteInput.chooseNoteDuration("pad-note-4")
            NoteInput.putNote("note-c")
            NoteInput.putTie()
            NoteInput.putTie()
            NoteInput.putTie()

            // slur
            //! TODO The problem with the finish of the slur, see issues/7985
//                NoteInput.chooseNoteDuration("pad-note-4")
//                NoteInput.putNote("note-c")
//                NoteInput.toggleSlur()
//                NoteInput.putNote("note-c")
//                NoteInput.putNote("note-c")
//                NoteInput.putNote("note-c")
//                NoteInput.toggleSlur()

            // articulation
            NoteInput.chooseNoteDuration("pad-note-8")
            NoteInput.toggleArticulation("add-marcato")
            NoteInput.putNote("note-c")
            NoteInput.toggleArticulation("add-sforzato") // `add-marcato` to `add-sforzato`
            NoteInput.putNote("note-c")
            NoteInput.toggleArticulation("add-sforzato") // `add-sforzato` off
            NoteInput.toggleArticulation("add-tenuto")
            NoteInput.putNote("note-c")
            NoteInput.toggleArticulation("add-tenuto") // `add-tenuto` off
            NoteInput.toggleArticulation("add-staccato")
            NoteInput.putNote("note-c")
            NoteInput.toggleArticulation("add-marcato") // `add-staccato` + `add-marcato`
            NoteInput.putNote("note-c")
            NoteInput.toggleArticulation("add-sforzato") // `add-staccato` + `add-sforzato`
            NoteInput.putNote("note-c")
            NoteInput.toggleArticulation("add-tenuto") // `add-staccato` + `add-tenuto + `add-sforzato`
            NoteInput.putNote("note-c")
            NoteInput.toggleArticulation("add-marcato") // `add-staccato` + `add-tenuto + `add-marcato`
            NoteInput.putNote("note-c")
            NoteInput.toggleArticulation("add-marcato") // `add-marcato` off
            NoteInput.toggleArticulation("add-tenuto") // `add-tenuto` off
            NoteInput.toggleArticulation("add-staccato") // `add-staccato` off

            NoteInput.chooseNoteDuration("pad-note-4")
            NoteInput.putNote("note-c")

            // tuplets
            NoteInput.putTuplet(3)
            //! TODO Not completed
        }},
        {name: "Save", func: function() {
            api.autobot.saveProject("TC3 Using note input toolbar.mscz")
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

function main()
{
    api.autobot.setInterval(1000)
    api.autobot.runTestCase(testCase)
}

function seeChanges()
{
    api.autobot.seeChanges()
}
