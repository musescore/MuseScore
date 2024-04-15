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
var Inspector = require("steps/Inspector.js")
var Score = require("steps/Score.js")

var testCase = {
    name: "TC6: Using inspector: Note",
    description: "Let's check the functionality of the inspector: Note",
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
        {name: "Open Inspector panel", func: function() {
            Inspector.openInspectorPanel()
        }},
        {name: "Use General Panel", skip: false, func: function() {
            // Normal visible note
            api.shortcuts.activate("C") // put note
            seeChanges()

            // Unvisible note
            api.shortcuts.activate("C") // put note
            Score.prevChord() // select note (move back cursor)
            Inspector.goToSection("General")
            api.navigation.down() // go to visible
            api.navigation.trigger() // make unvisible
            seeChanges()

            // Cue size note
            Score.nextChord()
            api.shortcuts.activate("C") // put note
            Score.prevChord() // select note (move back cursor)
            Inspector.goToSection("General")
            api.navigation.down() // go to visible
            api.navigation.right() // go to cue size
            api.navigation.trigger() // make cue size
            seeChanges()

            // Unvisible rest
            Score.nextChord()
            Inspector.goToSection("General")
            api.navigation.down() // go to visible
            api.navigation.trigger() // make unvisible
            seeChanges()
        }},
        {name: "Use Note Panel: Head", skip: false, func: function() {

            // Hide notehead
            Score.nextChord()
            api.shortcuts.activate("C") // put note
            Score.prevChord() // select note (move back cursor)
            api.navigation.triggerControl("NavigationLeftPanel", "Note", "HeadTab")
            navigationDown(3) // got to Hide notehead
            api.navigation.trigger()
            seeChanges()

            // Change notehead
            for (var nh = 0; nh < 3; ++nh) {
                Score.nextChord()
                api.shortcuts.activate("C") // put note
                Score.prevChord() // select note (move back cursor)
                Inspector.goToSection("Note")
                navigationDown(6 + nh) // got to notehead cell 2 + index
                api.navigation.trigger()
                seeChanges()
            }

            // Undo change notehead
            Score.prevChord()
            seeChanges()
            Score.prevChord()
            seeChanges()
            Inspector.goToSection("Note")
            navigationDown(5)
            seeChangesLong()
            api.navigation.trigger()
            seeChanges()

            // Dotted note position
            Score.nextMeasure()
            NoteInput.chooseNoteDuration("pad-note-8")
            NoteInput.toggleDot("pad-dot")
            api.shortcuts.activate("C")
            Score.prevChord() // select note (move back cursor)
            api.navigation.triggerControl("NavigationLeftPanel", "Note", "DottedNoteAuto")
            seeChanges()

            // Later we will reset this
            Score.nextChord()
            api.shortcuts.activate("C")
            Score.prevChord() // select note (move back cursor)
            api.navigation.triggerControl("NavigationLeftPanel", "Note", "DottedNoteUp")
            seeChanges()

            Score.nextChord()
            api.shortcuts.activate("C")
            Score.prevChord() // select note (move back cursor)
            api.navigation.triggerControl("NavigationLeftPanel", "Note", "DottedNoteDown")
            seeChanges()

            Score.nextChord()
            api.shortcuts.activate("C")
            Score.prevChord() // select note (move back cursor)
            api.navigation.triggerControl("NavigationLeftPanel", "Note", "DottedNoteUp")
            seeChanges()

            // Reset
            Score.prevChord()
            Score.prevChord()
            api.navigation.goToControl("NavigationLeftPanel", "Note", "DottedNoteReset")
            seeChangesLong()
            api.navigation.trigger()
        }},
        {name: "Use Note Panel: Stem", skip: false, func: function() {

            Score.nextMeasure()
            NoteInput.chooseNoteDuration("pad-note-8")
            api.shortcuts.activate("C")
            api.shortcuts.activate("C")
            Score.prevChord() // select note (move back cursor)
            api.navigation.triggerControl("NavigationLeftPanel", "Note", "StemTab") // show Stem tab
            api.navigation.triggerControl("NavigationLeftPanel", "Note", "HideStem") // hide stem
            seeChanges()

            // Stem direction
            // Up
            Score.nextChord()
            NoteInput.chooseNoteDuration("pad-note-4")
            api.shortcuts.activate("C")
            Score.prevChord() // select note (move back cursor)
            api.navigation.triggerControl("NavigationLeftPanel", "Note", "StemTab") // show Stem tab
            api.navigation.triggerControl("NavigationLeftPanel", "Note", "StemDirectionUp")
            seeChanges()

            // Later we will reset this
            Score.nextChord()
            api.shortcuts.activate("C")
            Score.prevChord() // select note (move back cursor)
            api.navigation.triggerControl("NavigationLeftPanel", "Note", "StemTab") // show Stem tab
            api.navigation.triggerControl("NavigationLeftPanel", "Note", "StemDirectionUp")
            seeChanges()

            // Down
            Score.nextChord()
            api.shortcuts.activate("C")
            Score.prevChord() // select note (move back cursor)
            Score.pitchDown(3)
            api.navigation.triggerControl("NavigationLeftPanel", "Note", "StemTab") // show Stem tab
            api.navigation.triggerControl("NavigationLeftPanel", "Note", "StemDirectionDown")
            seeChanges()

            // Reset (change to Auto)
            Score.prevChord()
            Score.prevChord()
            api.navigation.triggerControl("NavigationLeftPanel", "Note", "StemTab") // show Stem tab
            api.navigation.triggerControl("NavigationLeftPanel", "Note", "StemDirectionAuto")

            // Flag style
            // Stem down
            Score.nextMeasure()
            NoteInput.chooseNoteDuration("pad-note-8")
            api.shortcuts.activate("C")

            NoteInput.chooseNoteDuration("pad-note-16")
            Score.nextChord() // rest
            Score.nextChord()
            api.shortcuts.activate("C")

            NoteInput.chooseNoteDuration("pad-note-32")
            Score.nextChord() // rest
            Score.nextChord()
            api.shortcuts.activate("C")
            seeChanges()

            Score.prevChord() // select note (move back cursor)
            api.navigation.triggerControl("NavigationLeftPanel", "Note", "StemTab") // show Stem tab
            api.navigation.triggerControl("NavigationLeftPanel", "Note", "FlagStyleStraight")
            seeChangesVeryLong()

            api.navigation.triggerControl("NavigationLeftPanel", "Note", "FlagStyleTraditional")
            seeChangesVeryLong()

            api.navigation.triggerControl("NavigationLeftPanel", "Note", "FlagStyleStraight")
            seeChangesVeryLong()
        }},
        {name: "Use Note Panel: Beam", func: function() {

            Score.nextMeasure()
            NoteInput.chooseNoteDuration("pad-note-8")

            // Begin
            api.shortcuts.activate("C")
            api.shortcuts.activate("C")
            api.shortcuts.activate("C")
            api.shortcuts.activate("C")
            Score.prevChord() // select note (move back cursor)
            api.navigation.triggerControl("NavigationLeftPanel", "Note", "BeamTab") // show Beam tab
            api.navigation.triggerControl("NavigationLeftPanel", "Note", "BeamTypeBegin")
            seeChanges()

            // Middle
            Score.nextChord()
            api.shortcuts.activate("C")
            Score.prevChord() // select note (move back cursor)
            api.navigation.triggerControl("NavigationLeftPanel", "Note", "BeamTab") // show Beam tab
            api.navigation.triggerControl("NavigationLeftPanel", "Note", "BeamTypeMiddle")
            seeChanges()

            // None
            Score.nextChord()
            api.shortcuts.activate("C")
            seeChanges()
            Score.prevChord() // select note (move back cursor)
            api.navigation.triggerControl("NavigationLeftPanel", "Note", "BeamTab") // show Beam tab
            api.navigation.triggerControl("NavigationLeftPanel", "Note", "BeamTypeNone")
            seeChanges()

            // Force Horizontal
            // Not horizontal
            Score.nextMeasure()
            api.shortcuts.activate("C")
            api.shortcuts.activate("C")
            Score.prevChord() // select note (move back cursor)
            Score.pitchDown(3)
            seeChanges()

            // Horizontal
            Score.nextChord() // rest
            Score.nextChord()
            api.shortcuts.activate("C")
            api.shortcuts.activate("C")
            Score.prevChord() // select note (move back cursor)
            Score.pitchDown(3)
            seeChanges()
            api.navigation.triggerControl("NavigationLeftPanel", "Note", "BeamTab") // show Beam tab
            api.navigation.triggerControl("NavigationLeftPanel", "Note", "ForceHorizontal")
        }},
        {name: "Save", func: function() {
            api.autobot.saveProject("TC6_UsingInspector.mscz")
        }},
        {name: "Close", func: function() {
            api.dispatcher.dispatch("file-close")
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

function seeChanges()
{
    api.autobot.seeChanges()
}

function seeChangesLong()
{
    api.autobot.sleep(1000)
}

function seeChangesVeryLong()
{
    api.autobot.sleep(2000)
}

function navigationDown(count)
{
    for (var i = 0; i < count; ++i) {
        api.navigation.down()
    }
}

