/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited and others
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
var Instruments = require("steps/Instruments.js")
var Navigation = require("steps/Navigation.js")
var ScoreContent = require("steps/ScoreContent.js")
var Parts = require("steps/Parts.js")
var Home = require("steps/Home.js")
var Project = require("steps/Project.js")
var Score = require("steps/Score.js")
var NoteInput = require("steps/NoteInput.js")
var Palette = require("steps/Palette.js")
var UndoRedo = require("steps/UndoRedo.js")
var Commands = require("steps/Commands.js")
var engraving = require("MuseApi.Engraving")

const OPEN_PARTS_COMMAND = "command://notation/open-parts"
const ADD_CHORD_SYMBOL_COMMAND = "command://notation/add-chord-symbol"
const ADD_FRETBOARD_DIAGRAM_COMMAND = "command://notation/add-fretboard-diagram"
const INSERT_FRET_FRAME_COMMAND = "command://notation/insert-fretframe"
const DELETE_COMMAND = "command://notation/delete"

const GUITAR = "guitar-nylon"
const VOICE = "voice"
const PIANO = "piano"

//! NOTE The name of the guitar's standard staff in the Instruments panel (its tab staff is "[LINK] Tablature")
const GUITAR_STANDARD_STAFF_ROW = "Treble clef 8va bassa"

//! NOTE A whole 4/4 measure
const MEASURE_TICKS = 1920

function log(msg)
{
    api.log.info("[testflow] " + msg)
}

function expect(ok, msg)
{
    if (!ok) {
        api.testflow.fatal(msg)
    }
}

function expectEqual(expected, actual, what)
{
    expect(expected === actual, what + ": expected [" + expected + "], got [" + actual + "]")
    log(what + ": OK [" + actual + "]")
}

function waitForProject(timeoutMsec)
{
    for (var waited = 0; waited < timeoutMsec; waited += 100) {
        if (Project.isOpened() && engraving.curScore) {
            return true
        }
        api.testflow.sleep(100)
    }
    return false
}

//! NOTE The instruments of a score, top to bottom, e.g. "piano,guitar-nylon,voice"
function instrumentOrder(score)
{
    var ids = []
    for (var i = 0; i < score.parts.length; ++i) {
        ids.push(score.parts[i].instrumentId)
    }
    return ids.join(",")
}

//! NOTE Every staff of a score with its instrument, kind and visibility,
//! e.g. "voice shown, guitar-nylon HIDDEN, guitar-nylon tab shown"
function staffList(score)
{
    var staves = []
    for (var i = 0; i < score.staves.length; ++i) {
        var st = score.staves[i]
        //! NOTE A guitar part has its standard staff first, then the tab staff
        var isTab = st.part.hasTabStaff && st.part.hasPitchedStaff && i > 0
                    && score.staves[i - 1].part.instrumentId === st.part.instrumentId
        staves.push(st.part.instrumentId + (isTab ? " tab" : "") + (st.show ? " shown" : " HIDDEN"))
    }
    return staves.join(", ")
}

//! NOTE The first staff of an instrument in a score
function firstStaffOf(score, instrumentId)
{
    for (var i = 0; i < score.parts.length; ++i) {
        if (score.parts[i].instrumentId === instrumentId) {
            return Math.floor(score.parts[i].startTrack / 4)
        }
    }
    api.testflow.fatal("no " + instrumentId + " in the score")
    return -1
}

function guitarTrack()
{
    return firstStaffOf(engraving.curScore, GUITAR) * 4
}

//! NOTE The only part of the main score
function thePart()
{
    var parts = Parts.list()
    expect(parts.length === 1, "expected 1 part, got " + parts.length)
    return parts[0].score
}

function annotationAt(score, tick, type)
{
    for (var m = score.firstMeasure; m; m = m.nextMeasure) {
        for (var seg = m.firstSegment; seg; seg = seg.nextInMeasure) {
            if (seg.tick !== tick) {
                continue
            }
            var anns = seg.annotations
            for (var i = 0; i < anns.length; ++i) {
                if (anns[i].type === type) {
                    return anns[i]
                }
            }
        }
    }
    return null
}

//! NOTE Fretboard diagrams of a score with their chords, e.g. "C@0, G@1920"
function fretDiagrams(score)
{
    var found = []
    for (var m = score.firstMeasure; m; m = m.nextMeasure) {
        for (var seg = m.firstSegment; seg; seg = seg.nextInMeasure) {
            var anns = seg.annotations
            for (var i = 0; i < anns.length; ++i) {
                if (anns[i].type === engraving.Element.FRET_DIAGRAM) {
                    found.push(anns[i].harmonyPlainText + "@" + seg.tick)
                }
            }
        }
    }
    return found.join(", ")
}

function keyOf(score)
{
    var cursor = score.newCursor()
    cursor.track = 0
    cursor.rewind(0)
    return cursor.keySignature
}

function timeSigOf(score)
{
    var ts = score.firstMeasure ? score.firstMeasure.timesigActual : null
    return ts ? ts.numerator + "/" + ts.denominator : "?"
}

//! NOTE What this test changes in a score, as one line, so two states can be compared
function stateOf(score)
{
    return "instruments " + instrumentOrder(score) + "; staves " + staffList(score)
           + "; fretboard diagrams [" + fretDiagrams(score) + "]; key " + keyOf(score) + "; time " + timeSigOf(score)
}

//! NOTE The row of a named control in the Instruments panel
function rowOf(name)
{
    var controls = api.navigation.controls("NavigationLeftPanel", "LayoutPanelTree")
    for (var i = 0; i < controls.length; ++i) {
        if (controls[i].name === name) {
            return controls[i].index.row
        }
    }
    api.testflow.fatal("no row named '" + name + "' in the Instruments panel")
    return -1
}

//! NOTE Clicks the eye (visibility) button of a row in the Instruments panel.
//! Column 0 is the row's name, column 1 its eye button
function clickEye(name)
{
    api.navigation.goToControl("NavigationLeftPanel", "LayoutPanelTree", [rowOf(name), 1], 2000)
    api.navigation.trigger()
    api.testflow.sleep(1000)
}

//! NOTE Clicks the expand button (column 2) of an instrument row, to show its staves
function expandInstrument(name)
{
    api.navigation.goToControl("NavigationLeftPanel", "LayoutPanelTree", [rowOf(name), 2], 2000)
    api.navigation.trigger()
    api.testflow.sleep(500)
}

//! NOTE Adds one chord symbol: type it, then Esc
function addChordSymbol(tick, text)
{
    Score.focusIn()
    ScoreContent.selectAt(guitarTrack(), tick)
    api.dispatcher.dispatch(ADD_CHORD_SYMBOL_COMMAND)
    api.testflow.sleep(300)
    api.keyboard.text(text)
    api.shortcuts.activate("Esc")
    api.testflow.sleep(300)
}

//! NOTE A fretboard diagram is added to the selected chord symbol and shows that chord
function addFretDiagram(tick)
{
    Score.focusIn()
    var harmony = annotationAt(engraving.curScore, tick, engraving.Element.HARMONY)
    expect(harmony, "no chord symbol at tick " + tick + " to add a fretboard diagram to")
    engraving.curScore.selection.select(harmony)
    api.testflow.seeChanges()
    api.dispatcher.dispatch(ADD_FRETBOARD_DIAGRAM_COMMAND)
    api.testflow.sleep(500)
    api.shortcuts.activate("Esc")
    api.testflow.sleep(200)
}

function applyPaletteCell(palette, cell)
{
    Score.focusIn()
    ScoreContent.selectAt(guitarTrack(), 0)
    api.testflow.seeChanges()
    Palette.togglePalette(palette)
    api.testflow.seeChanges()
    expect(Navigation.hasControl("NavigationLeftPanel", "PalettesTree", cell), "no '" + cell + "' in the " + palette + " palette")
    api.navigation.goToControl("NavigationLeftPanel", "PalettesTree", cell, 2000)
    api.navigation.trigger()
    api.testflow.sleep(500)
    Palette.togglePalette(palette)
}

//! NOTE The chord lines of a staff (pitches and lengths), without the staff's other content
function chordLines(score, staff)
{
    return ScoreContent.contentLines(score, staff).filter(function(l) { return l.indexOf("chord") !== -1 })
}

//! NOTE What the test expects at the end, before undo all
var finalScoreState = ""
var finalPartState = ""
var finalScoreLines = []

var testCase = {
    name: "TC14: Guitar + Tablature with a custom part",
    description: "Start from the Guitar + Tablature template, add Voice and Piano, make a custom part with Voice and the guitar tab only, "
                 + "reorder instruments in the part and in the score, add notes, chord symbols, fretboard diagrams and a legend, "
                 + "change key and time signatures, delete and add a fretboard diagram, then undo all and redo all",
    steps: [
        {name: "Close score (if opened) and go to home to start", func: function() {
            Project.closeIfOpened()
            Home.goToHome()
        }},
        {name: "Open New Score Dialog", func: function() {
            NewScore.openNewScoreDialog()
        }},
        {name: "Choose the Guitar + Tablature template", func: function() {
            NewScore.selectTab("templates")
            //! NOTE Moving to a category or template only focuses it, so trigger both
            Navigation.triggerControl("NewScoreDialog", "Category", "Solo")
            api.testflow.sleep(500)
            Navigation.triggerControl("NewScoreDialog", "Template", "Guitar + Tablature")
            api.testflow.sleep(300)
        }},
        {name: "Create the score", func: function() {
            NewScore.done()
            expect(waitForProject(5000), "no score is opened")
            expectEqual(GUITAR + " shown, " + GUITAR + " tab shown", staffList(engraving.curScore), "template staves")
        }},
        {name: "Add Voice and Piano", func: function() {
            Instruments.openAddInstrumentsDialog()
            Instruments.chooseInstrument("Vocals", "Voice")
            Instruments.chooseInstrument("Keyboards", "Piano")
            Instruments.doneAddInstruments()
            api.testflow.sleep(1000)
            //! NOTE New instruments go in score order
            expectEqual([PIANO, GUITAR, VOICE].join(","), instrumentOrder(engraving.curScore), "score instruments after adding")
        }},
        {name: "Create a new part and open it", func: function() {
            api.dispatcher.dispatch(OPEN_PARTS_COMMAND)
            Navigation.popupOpened("PartsControlPanel")
            Navigation.goToControlInPopup("PartsControlPanel", "CreateNewPartButton")
            api.navigation.trigger()
            api.testflow.sleep(500)
            Navigation.goToControlInPopup("ButtonBox", "Open selected")
            api.navigation.trigger()
            api.testflow.sleep(1500)
            //! NOTE A new part starts empty, and opens in its own tab
            expectEqual(1, Parts.list().length, "parts after creating one")
            expectEqual(0, engraving.curScore.nstaves, "staves in the new (current) part")
        }},
        {name: "Show Voice and Guitar in the part with their eye buttons", func: function() {
            Instruments.openInstrumentsPanel()
            clickEye("Voice")
            clickEye("Guitar")
            expectEqual(GUITAR + " shown, " + GUITAR + " tab shown, " + VOICE + " shown", staffList(engraving.curScore), "part staves")
        }},
        {name: "Hide the guitar standard staff in the part", func: function() {
            expandInstrument("Guitar")
            clickEye(GUITAR_STANDARD_STAFF_ROW)
            expectEqual(GUITAR + " HIDDEN, " + GUITAR + " tab shown, " + VOICE + " shown", staffList(engraving.curScore),
                        "part staves after hiding the guitar standard staff")
        }},
        {name: "Move Voice up in the part", func: function() {
            Instruments.goToInstrument("Voice")
            api.navigation.trigger() // select the instrument
            //! NOTE Up is only enabled once the selection has reached the header
            api.testflow.sleep(500)
            Navigation.triggerControl("NavigationLeftPanel", "InstrumentsHeader", "Up")
            api.testflow.sleep(500)
            expectEqual(VOICE + " shown, " + GUITAR + " HIDDEN, " + GUITAR + " tab shown", staffList(engraving.curScore),
                        "part staves after moving Voice up")
            Parts.goToMainScore()
            //! NOTE The part's order doesn't change the score
            expectEqual([PIANO, GUITAR, VOICE].join(","), instrumentOrder(engraving.curScore), "score instruments after reordering the part")
        }},
        {name: "Move Voice up in the score", func: function() {
            Instruments.goToInstrument("Voice")
            api.navigation.trigger() // select the instrument
            api.testflow.sleep(500)
            Navigation.triggerControl("NavigationLeftPanel", "InstrumentsHeader", "Up")
            api.testflow.sleep(500)
            expectEqual([PIANO, VOICE, GUITAR].join(","), instrumentOrder(engraving.curScore), "score instruments after moving Voice up")
            //! NOTE The score's order doesn't change the part
            expectEqual(VOICE + " shown, " + GUITAR + " HIDDEN, " + GUITAR + " tab shown", staffList(thePart()),
                        "part staves after reordering the score")
        }},
        {name: "Put guitar notes: C E G C G B D G", func: function() {
            Score.focusIn()
            ScoreContent.selectAt(guitarTrack(), 0)
            NoteInput.chooseDefaultMode()
            NoteInput.chooseNoteDuration(Commands.SET_DURATION_QUARTER_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_E_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_G_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_G_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_B_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_D_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_G_COMMAND)
            NoteInput.turnoffDefaultMode()
            api.testflow.seeChanges()

            var staff = firstStaffOf(engraving.curScore, GUITAR)
            var standard = chordLines(engraving.curScore, staff)
            expectEqual(8, standard.length, "guitar notes")
            //! NOTE The tab staff is linked to the standard staff, so it has the same notes
            ScoreContent.expectSameLines(standard, chordLines(engraving.curScore, staff + 1), "tab staff notes")
        }},
        {name: "Add chord symbols C and G", func: function() {
            addChordSymbol(0, "C")
            addChordSymbol(MEASURE_TICKS, "G")
            var c = annotationAt(engraving.curScore, 0, engraving.Element.HARMONY)
            var g = annotationAt(engraving.curScore, MEASURE_TICKS, engraving.Element.HARMONY)
            expectEqual("C", c ? c.plainText : "none", "chord symbol at the first measure")
            expectEqual("G", g ? g.plainText : "none", "chord symbol at the second measure")
        }},
        {name: "Add fretboard diagrams to both chords", func: function() {
            addFretDiagram(0)
            addFretDiagram(MEASURE_TICKS)
            expectEqual("C@0, G@" + MEASURE_TICKS, fretDiagrams(engraving.curScore), "fretboard diagrams in the score")
            expectEqual("C@0, G@" + MEASURE_TICKS, fretDiagrams(thePart()), "fretboard diagrams in the part")
        }},
        {name: "Add a fretboard diagram legend", func: function() {
            ScoreContent.selectAt(guitarTrack(), 0)
            api.dispatcher.dispatch(INSERT_FRET_FRAME_COMMAND)
            api.testflow.sleep(500)
            var sel = engraving.curScore.selection.elements
            expect(sel.length > 0 && sel[0].type === engraving.Element.FBOX,
                   "after adding a legend, expected the legend (FBox) to be selected, got " + (sel.length ? sel[0].name : "nothing"))
            log("legend added: OK")
        }},
        {name: "Change the key signature to G major", func: function() {
            applyPaletteCell("Key signatures", "G major / E minor")
            expectEqual(1, keyOf(engraving.curScore), "key in the score (sharps)")
            expectEqual(1, keyOf(thePart()), "key in the part (sharps)")
        }},
        {name: "Change the time signature to 3/4", func: function() {
            applyPaletteCell("Time signatures", "3/4")
            expectEqual("3/4", timeSigOf(engraving.curScore), "time signature in the score")
            expectEqual("3/4", timeSigOf(thePart()), "time signature in the part")
        }},
        {name: "Delete the first fretboard diagram and add it again", func: function() {
            //! NOTE Read the positions again: the time signature change could move them
            var before = fretDiagrams(engraving.curScore)
            var fd = annotationAt(engraving.curScore, 0, engraving.Element.FRET_DIAGRAM)
            expect(fd, "no fretboard diagram at the start to delete")
            Score.focusIn()
            engraving.curScore.selection.select(fd)
            api.testflow.seeChanges()
            api.dispatcher.dispatch(DELETE_COMMAND)
            api.testflow.sleep(500)
            var afterDelete = fretDiagrams(engraving.curScore)
            expect(afterDelete.split(", ").length === 1 && afterDelete.indexOf("@0") === -1,
                   "after deleting the first fretboard diagram, got [" + afterDelete + "]")
            log("fretboard diagram deleted: OK [" + afterDelete + "]")
            addFretDiagram(0)
            expectEqual(before, fretDiagrams(engraving.curScore), "fretboard diagrams after adding it again")
            expectEqual(before, fretDiagrams(thePart()), "fretboard diagrams in the part after adding it again")
        }},
        {name: "Check the part matches the score", func: function() {
            var score = engraving.curScore
            var part = thePart()
            expectEqual(VOICE + " shown, " + GUITAR + " HIDDEN, " + GUITAR + " tab shown", staffList(part), "part staves")
            var voice = firstStaffOf(score, VOICE)
            var guitar = firstStaffOf(score, GUITAR)
            ScoreContent.expectSameLines(ScoreContent.contentLines(score, voice), ScoreContent.contentLines(part, 0), "part Voice staff")
            ScoreContent.expectSameLines(ScoreContent.contentLines(score, guitar), ScoreContent.contentLines(part, 1), "part guitar staff")
            ScoreContent.expectSameLines(ScoreContent.contentLines(score, guitar + 1), ScoreContent.contentLines(part, 2), "part guitar tab staff")

            finalScoreState = stateOf(score)
            finalPartState = stateOf(part)
            finalScoreLines = ScoreContent.contentLines(score)
            log("score before undo all: " + finalScoreState)
            log("part before undo all: " + finalPartState)
        }},
        {name: "Undo all", func: function() {
            Parts.goToMainScore()
            //! NOTE The number of steps depends on how commands are grouped, so it is not checked
            UndoRedo.undoAll()
            api.testflow.seeChanges()
            expectEqual(0, Parts.list().length, "parts after undo all")
            expectEqual(GUITAR + " shown, " + GUITAR + " tab shown", staffList(engraving.curScore), "staves after undo all")
            expectEqual("", fretDiagrams(engraving.curScore), "fretboard diagrams after undo all")
            expectEqual(0, keyOf(engraving.curScore), "key after undo all")
            expectEqual("4/4", timeSigOf(engraving.curScore), "time signature after undo all")
        }},
        {name: "Redo all", func: function() {
            UndoRedo.redoAll()
            api.testflow.seeChanges()
            expectEqual(finalScoreState, stateOf(engraving.curScore), "score after redo all")
            expectEqual(finalPartState, stateOf(thePart()), "part after redo all")
            ScoreContent.expectSameLines(finalScoreLines, ScoreContent.contentLines(engraving.curScore), "score content after redo all")
        }}
    ]
};

function main()
{
    api.testflow.setInterval(500)
    api.testflow.runTestCase(testCase)
}
