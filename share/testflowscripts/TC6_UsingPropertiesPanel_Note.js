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

var Navigation = require("steps/Navigation.js")
var NewScore = require("steps/NewScore.js")
var NoteInput = require("steps/NoteInput.js")
var PropertiesPanel = require("steps/PropertiesPanel.js")
var Score = require("steps/Score.js")
var Home = require("steps/Home.js")
var Project = require("steps/Project.js")
var Commands = require("steps/Commands.js")

var testCase = {
    name: "TC6: Using properties panel: Note",
    description: "Let's check the functionality of the properties panel: Note",
    steps: [
        {name: "Close score (if opened) and go to home to start", func: function() {
            Project.closeIfOpened()
            Navigation.triggerControl("TopTool", "MainToolBar", "Home")
        }},
        {name: "Open New Score Dialog", func: function() {
            NewScore.openNewScoreDialog()
        }},
        {name: "Create score", func: function() {
            NewScore.chooseInstrument("Woodwinds", "Flute")
            NewScore.done()
        }},
        {name: "Open Properties panel", func: function() {
            PropertiesPanel.openPropertiesPanel()
        }},
        {name: "Use General Panel", skip: false, func: function() {
            // Normal visible note
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND) // put note
            seeChanges()

            // Unvisible note
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND) // put note
            Score.prevChord() // select note (move back cursor)
            PropertiesPanel.goToSection("General")
            api.navigation.down() // go to visible
            api.navigation.trigger() // make unvisible
            seeChanges()

            // Cue size note
            Score.nextChord()
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND) // put note
            Score.prevChord() // select note (move back cursor)
            PropertiesPanel.goToSection("General")
            api.navigation.down() // go to visible
            api.navigation.right() // go to cue size
            api.navigation.trigger() // make cue size
            seeChanges()

            // Unvisible rest
            Score.nextChord()
            PropertiesPanel.goToSection("General")
            api.navigation.down() // go to visible
            api.navigation.trigger() // make unvisible
            seeChanges()
        }},
        {name: "Use Note Panel: Head", skip: false, func: function() {

            // Hide notehead
            Score.nextChord()
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND) // put note
            Score.prevChord() // select note (move back cursor)
            Navigation.triggerControl("NavigationLeftPanel", "Note", "HeadTab")
            navigationDown(3) // got to Hide notehead
            api.navigation.trigger()
            seeChanges()

            // Change notehead
            for (var nh = 0; nh < 3; ++nh) {
                Score.nextChord()
                NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND) // put note
                Score.prevChord() // select note (move back cursor)
                PropertiesPanel.goToSection("Note")
                navigationDown(6 + nh) // got to notehead cell 2 + index
                api.navigation.trigger()
                seeChanges()
            }

            // Undo change notehead
            Score.prevChord()
            seeChanges()
            Score.prevChord()
            seeChanges()
            PropertiesPanel.goToSection("Note")
            navigationDown(5)
            seeChangesLong()
            api.navigation.trigger()
            seeChanges()

            // Dotted note position
            Score.nextMeasure()
            NoteInput.chooseNoteDuration(Commands.SET_DURATION_EIGHTH_COMMAND)
            NoteInput.toggleDot(Commands.TOGGLE_DOT_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            Score.prevChord() // select note (move back cursor)
            Navigation.triggerControl("NavigationLeftPanel", "Note", "DottedNoteAuto")
            seeChanges()

            // Later we will reset this
            Score.nextChord()
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            Score.prevChord() // select note (move back cursor)
            Navigation.triggerControl("NavigationLeftPanel", "Note", "DottedNoteUp")
            seeChanges()

            Score.nextChord()
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            Score.prevChord() // select note (move back cursor)
            Navigation.triggerControl("NavigationLeftPanel", "Note", "DottedNoteDown")
            seeChanges()

            Score.nextChord()
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            Score.prevChord() // select note (move back cursor)
            Navigation.triggerControl("NavigationLeftPanel", "Note", "DottedNoteUp")
            seeChanges()

            // Reset
            Score.prevChord()
            Score.prevChord()
            Navigation.goToControl("NavigationLeftPanel", "Note", "DottedNoteReset")
            seeChangesLong()
            api.navigation.trigger()
        }},
        {name: "Use Note Panel: Stem", skip: false, func: function() {

            Score.nextMeasure()
            NoteInput.chooseNoteDuration(Commands.SET_DURATION_EIGHTH_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            Score.prevChord() // select note (move back cursor)
            Navigation.triggerControl("NavigationLeftPanel", "Note", "StemTab") // show Stem tab
            Navigation.triggerControl("NavigationLeftPanel", "Note", "Stemless") // hide stem
            seeChanges()

            // Stem direction
            // Up
            Score.nextChord()
            NoteInput.chooseNoteDuration(Commands.SET_DURATION_QUARTER_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            Score.prevChord() // select note (move back cursor)
            Navigation.triggerControl("NavigationLeftPanel", "Note", "StemTab") // show Stem tab
            Navigation.triggerControl("NavigationLeftPanel", "Note", "StemDirectionUp")
            seeChanges()

            // Later we will reset this
            Score.nextChord()
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            Score.prevChord() // select note (move back cursor)
            Navigation.triggerControl("NavigationLeftPanel", "Note", "StemTab") // show Stem tab
            Navigation.triggerControl("NavigationLeftPanel", "Note", "StemDirectionUp")
            seeChanges()

            // Down
            Score.nextChord()
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            Score.prevChord() // select note (move back cursor)
            Score.pitchDown(3)
            Navigation.triggerControl("NavigationLeftPanel", "Note", "StemTab") // show Stem tab
            Navigation.triggerControl("NavigationLeftPanel", "Note", "StemDirectionDown")
            seeChanges()

            // Reset (change to Auto)
            Score.prevChord()
            Score.prevChord()
            Navigation.triggerControl("NavigationLeftPanel", "Note", "StemTab") // show Stem tab
            Navigation.triggerControl("NavigationLeftPanel", "Note", "StemDirectionAuto")

            // Flag style
            // Stem down
            Score.nextMeasure()
            NoteInput.chooseNoteDuration(Commands.SET_DURATION_EIGHTH_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)

            NoteInput.chooseNoteDuration(Commands.SET_DURATION_16TH_COMMAND)
            Score.nextChord() // rest
            Score.nextChord()
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)

            NoteInput.chooseNoteDuration(Commands.SET_DURATION_32ND_COMMAND)
            Score.nextChord() // rest
            Score.nextChord()
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            seeChanges()

            Score.prevChord() // select note (move back cursor)
            Navigation.triggerControl("NavigationLeftPanel", "Note", "StemTab") // show Stem tab
            Navigation.triggerControl("NavigationLeftPanel", "Note", "FlagStyleStraight")
            seeChangesVeryLong()

            Navigation.triggerControl("NavigationLeftPanel", "Note", "FlagStyleTraditional")
            seeChangesVeryLong()

            Navigation.triggerControl("NavigationLeftPanel", "Note", "FlagStyleStraight")
            seeChangesVeryLong()
        }},
        {name: "Use Note Panel: Beam", func: function() {

            Score.nextMeasure()
            NoteInput.chooseNoteDuration(Commands.SET_DURATION_EIGHTH_COMMAND)

            // Begin
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            Score.prevChord() // select note (move back cursor)
            Navigation.triggerControl("NavigationLeftPanel", "Note", "BeamTab") // show Beam tab
            Navigation.triggerControl("NavigationLeftPanel", "Note", "BeamTypeBreak beam left")
            seeChanges()

            // Middle
            Score.nextChord()
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            Score.prevChord() // select note (move back cursor)
            Navigation.triggerControl("NavigationLeftPanel", "Note", "BeamTab") // show Beam tab
            Navigation.triggerControl("NavigationLeftPanel", "Note", "BeamTypeJoin beams")
            seeChanges()

            // None
            Score.nextChord()
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            seeChanges()
            Score.prevChord() // select note (move back cursor)
            Navigation.triggerControl("NavigationLeftPanel", "Note", "BeamTab") // show Beam tab
            Navigation.triggerControl("NavigationLeftPanel", "Note", "BeamTypeNo beam")
            seeChanges()

            // Force Horizontal
            // Not horizontal
            Score.nextMeasure()
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            Score.prevChord() // select note (move back cursor)
            Score.pitchDown(3)
            seeChanges()

            // Horizontal
            Score.nextChord() // rest
            Score.nextChord()
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            Score.prevChord() // select note (move back cursor)
            Score.pitchDown(3)
            seeChanges()
            Navigation.triggerControl("NavigationLeftPanel", "Note", "BeamTab") // show Beam tab
            Navigation.triggerControl("NavigationLeftPanel", "Note", "ForceHorizontal")
        }},
        {name: "Save", func: function() {
            api.testflow.saveProject("TC6_UsingPropertiesPanel.mscz")
        }},
        {name: "Close", func: function() {
            api.dispatcher.dispatch(Commands.PROJECT_CLOSE_COMMAND)
        }},
        {name: "Home", func: function() {
            Navigation.triggerControl("TopTool", "MainToolBar", "Home")
        }},
        {name: "Open last", func: function() {
            Home.openLastProject()
        }}
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
    api.testflow.sleep(1000)
}

function seeChangesVeryLong()
{
    api.testflow.sleep(2000)
}

function navigationDown(count)
{
    for (var i = 0; i < count; ++i) {
        api.navigation.down()
    }
}

