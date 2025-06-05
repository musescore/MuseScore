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
var Palette = require("steps/Palette.js")
var Score = require("steps/Score.js")
var Home = require("steps/Home.js")

var testCase = {
    name: "TC4: Using palettes",
    description: "Let's check the functionality of the palettes",
    steps: [
        {name: "Close score (if opened) and go to home to start", func: function() {
            api.dispatcher.dispatch("file-close")
            Home.goToHome()
        }},
        {name: "Open New Score Dialog", func: function() {
            NewScore.openNewScoreDialog()
        }},
        {name: "Create score", func: function() {
            NewScore.chooseInstrument("Keyboards", "Piano")
            NewScore.done()
        }},
        {name: "Confirm about all Palettes collapsed", func: function() {
            api.autobot.confirm("All Palettes should be collapsed")
        }},
        {name: "Change Clefs", skip: false, func: function() {
            //! NOTE First, we need to select the current Clef
            api.navigation.goToControl("NotationView", "ScoreView", "Score")
            Score.firstElement()
            api.shortcuts.activate("Alt+Right") // select Clef
            seeChanges()

            //! NOTE Return to Palettes and expand Clefs
            Palette.togglePalette("Clefs")
            seeChanges()
            applyCellByCell(24, 0) //24
        }},
        {name: "Change Key signatures", skip: false, func: function() {
            //! NOTE To change Key sig, we need to select the first Chord on score
            api.navigation.goToControl("NotationView", "ScoreView", "Score")
            api.shortcuts.activate("Alt+Right") // select Time sig
            api.shortcuts.activate("Alt+Right") // select Rest

            //! NOTE Return to Palettes and expand Key signatures
            Palette.togglePalette("Key signatures")
            seeChanges()
            applyCellByCell(16, 1) //16 (14 - no key sig)
        }},
        {name: "Change Time signatures", skip: true, func: function() {
            Palette.togglePalette("Time signatures")
            seeChanges()
            applyCellByCell(17, 2)
        }},
        {name: "Add Accidentals", skip: false, func: function() {
            Palette.togglePalette("Accidentals") // open
            putNoteAndApplyCell(11)
            Palette.togglePalette("Accidentals") // close
        }},
        {name: "Add Articulations", skip: false, func: function() {
            Palette.togglePalette("Articulations") // open
            putNoteAndApplyCell(34)
            Palette.togglePalette("Articulations") // close
        }},
        {name: "Change Noteheads", skip: false, func: function() {
            Palette.togglePalette("Noteheads") // open
            putNoteAndApplyCell(24)
            Palette.togglePalette("Noteheads") // close
        }},
        {name: "Add Lines", skip: false, func: function() {
            Score.nextChord()
            Score.appendMeasures(35)
            seeChanges()

            Palette.togglePalette("Lines") // open
            applyLines(35)
            Palette.togglePalette("Lines") // close
        }},
        {name: "Change Barlines", skip: false, func: function() {
            Score.nextChord()

            Palette.togglePalette("Barlines") // open
            applyBarlines(15)
            Palette.togglePalette("Barlines") // close
        }},
        {name: "Add Tempo", skip: false, func: function() {
            Score.nextChord()
            Score.appendMeasures(28 * 2)

            Palette.togglePalette("Tempo") // open
            applyTempo(28)
            Palette.togglePalette("Tempo") // close
        }},
        {name: "Add Dynamics", skip: false, func: function() {
            Score.nextChord()
            Score.appendMeasures(24/2)

            Palette.togglePalette("Dynamics") // open
            applyDynamics(24)
            Palette.togglePalette("Dynamics") // close
        }},
        {name: "Add Arpeggios", skip: false, func: function() {
            Score.nextChord()

            Palette.togglePalette("Arpeggios & glissandi") // open
            applyArpeggios(6)
            Palette.togglePalette("Arpeggios & glissandi") // close
        }},
        {name: "Add Ornaments", skip: false, func: function() {
            Score.nextChord()
            Score.appendMeasures(16/2)

            Palette.togglePalette("Ornaments") // open
            applyOrnaments(16)
            Palette.togglePalette("Ornaments") // close
        }},
        {name: "Save", func: function() {
            api.autobot.saveProject("TC4_UsingPalettes.mscz")
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
    api.autobot.setInterval(500)
    api.autobot.runTestCase(testCase)
}

function seeChanges()
{
    api.autobot.seeChanges()
}

function applyCellByCell(count, keep_index)
{
    // apply
    for (var i = 0; i < count; i++) {
        api.navigation.down()
        api.navigation.trigger()
        seeChanges()
    }

    // back

    for (var r = (count - 1); r >= 0; r--) {
        api.navigation.up()
        api.autobot.seeChanges(100)

        // keep
        if ((keep_index + 1) === r) {
            api.navigation.trigger()
        }
    }

    // collapse
    api.navigation.trigger()
}

function putNoteAndApplyCell(count)
{
    for (var i = 0; i < count; i++) {
        api.shortcuts.activate("C")
        api.navigation.down()
        api.navigation.trigger()
        seeChanges()
    }
}

function applyLines(count)
{
    for (var i = 0; i < count; i++) {
        api.shortcuts.activate("C")
        api.navigation.down()
        api.navigation.trigger()
        api.shortcuts.activate("C")
        api.shortcuts.activate("C")
        api.shortcuts.activate("C")
        seeChanges()
    }
}

function applyBarlines(count)
{
    for (var i = 0; i < count; i++) {
        api.shortcuts.activate("C")
        api.shortcuts.activate("C")
        api.navigation.down()
        api.navigation.trigger()
        seeChanges()
    }
}

function applyTempo(count)
{
    for (var i = 0; i < count; i++) {
        api.shortcuts.activate("C")
        api.navigation.down()
        api.navigation.trigger()
        api.shortcuts.activate("C")
        api.shortcuts.activate("C")
        api.shortcuts.activate("C")
        api.shortcuts.activate("C")
        api.shortcuts.activate("C")
        api.shortcuts.activate("C")
        api.shortcuts.activate("C")
        seeChanges()
    }
}

function applyDynamics(count)
{
    for (var i = 0; i < count; i++) {
        api.shortcuts.activate("C")
        api.navigation.down()
        api.navigation.trigger()
        api.shortcuts.activate("C")
        seeChanges()
    }
}

function applyArpeggios(count)
{
    for (var i = 0; i < count; i++) {
        api.shortcuts.activate("C")
        api.navigation.down()
        api.navigation.trigger()
        api.shortcuts.activate("Shift+C")
        seeChanges()
    }
}

function applyOrnaments(count)
{
    for (var i = 0; i < count; i++) {
        api.shortcuts.activate("C")
        api.navigation.down()
        api.navigation.trigger()
        api.shortcuts.activate("C")
        seeChanges()
    }
}

