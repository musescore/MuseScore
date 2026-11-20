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
var NoteInput = require("steps/NoteInput.js")
var Score = require("steps/Score.js")
var ScoreContent = require("steps/ScoreContent.js")
var Parts = require("steps/Parts.js")
var Home = require("steps/Home.js")
var Project = require("steps/Project.js")
var Commands = require("steps/Commands.js")
var engraving = require("MuseApi.Engraving")

const FILE_NAME = "TC12_ScorePartsSaveReopen.mscz"

const ADD_TRIPLET_COMMAND = "command://notation/add-tuplet/triplet"
const ADD_SLUR_COMMAND = "command://notation/add-slur"
const ADD_HAIRPIN_COMMAND = "command://notation/add-hairpin"
const ADD_DYNAMIC_COMMAND = "command://notation/add-dynamic"
const ADD_LYRICS_COMMAND = "command://notation/add-lyrics"
const ADD_STAFF_TEXT_COMMAND = "command://notation/add-text/staff"
const ADD_TEMPO_COMMAND = "command://notation/add-tempo"

//! NOTE The flute notes that should be in the score after note input:
//! tick, length (quarter = 480), pitch class, tied to the next note, in a tuplet
const EXPECTED_NOTES = [
    { tick: 0,    len: 480, pc: 0,  tie: false, tuplet: false }, // C, staccato
    { tick: 480,  len: 480, pc: 2,  tie: false, tuplet: false }, // D, tenuto
    { tick: 960,  len: 480, pc: 4,  tie: true,  tuplet: false }, // E tied...
    { tick: 1440, len: 480, pc: 4,  tie: false, tuplet: false }, // ...to E
    { tick: 1920, len: 160, pc: 5,  tie: false, tuplet: true },  // F G A triplet eighths
    { tick: 2080, len: 160, pc: 7,  tie: false, tuplet: true },
    { tick: 2240, len: 160, pc: 9,  tie: false, tuplet: true },
    { tick: 2400, len: 480, pc: 11, tie: false, tuplet: false }, // B C D
    { tick: 2880, len: 480, pc: 0,  tie: false, tuplet: false },
    { tick: 3360, len: 480, pc: 2,  tie: false, tuplet: false },
    { tick: 3840, len: 960, pc: 4,  tie: false, tuplet: false }, // E F halves
    { tick: 4800, len: 960, pc: 5,  tie: false, tuplet: false }
]

//! NOTE What was saved, to compare after reopening
var savedScore = []
var savedParts = []

//! NOTE The text edit starts asynchronously, the typed keys must come after it is started
function startTextEdit(command)
{
    api.dispatcher.dispatch(command)
    api.testflow.sleep(300)
}

function endTextEdit()
{
    api.shortcuts.activate("Esc")
    api.testflow.sleep(200)
}

//! NOTE Puts the keyboard focus on the score, then selects the note at `tick` on the flute staff
function selectFluteNote(tick)
{
    Score.focusIn()
    ScoreContent.selectAt(0, tick)
    api.testflow.seeChanges()
}

//! NOTE Selects the flute notes from startTick up to (not including) endTick, the way a user does:
//! click the first note, then Shift+Right ("add next chord to selection") until the range is covered.
//! Selection.selectRange() from the API sets the range but not the list of selected notes,
//! and commands like Add slur work from that list, so they do nothing after it.
const ADD_NEXT_CHORD_TO_SELECTION_COMMAND = "command://notation/add-to-selection/next-chord"
const MAX_RANGE_STEPS = 16

function selectFluteRange(startTick, endTick)
{
    selectFluteNote(startTick)
    var sel = engraving.curScore.selection
    for (var i = 0; i < MAX_RANGE_STEPS; ++i) {
        if (sel.isRange && sel.endSegment && sel.endSegment.tick >= endTick) {
            break
        }
        api.dispatcher.dispatch(ADD_NEXT_CHORD_TO_SELECTION_COMMAND)
        api.testflow.sleep(100)
        sel = engraving.curScore.selection
    }
    var start = sel.startSegment ? sel.startSegment.tick : -1
    var end = sel.endSegment ? sel.endSegment.tick : -1
    if (!sel.isRange || start !== startTick || end !== endTick) {
        api.testflow.fatal("could not select ticks " + startTick + "-" + endTick + ", got range " + sel.isRange
                           + " ticks " + start + "-" + end)
    }
    api.testflow.seeChanges()
}

//! NOTE Waits until a project is open (loading may take a moment)
function waitForProject(timeoutMsec)
{
    for (var waited = 0; waited < timeoutMsec; waited += 100) {
        if (Project.isOpened() && engraving.curScore) {
            return true
        }
        api.testflow.sleep(100)
    }
    return Project.isOpened() && engraving.curScore
}

//! NOTE Every part should match its staff in the main score (with the system items)
function checkPartsMatchScore(what)
{
    var parts = Parts.list()
    if (parts.length !== 2) {
        api.testflow.fatal(what + ": expected 2 parts, got " + parts.length)
    }
    for (var i = 0; i < parts.length; ++i) {
        var expected = ScoreContent.contentLines(engraving.curScore, i)
        var actual = ScoreContent.contentLines(parts[i].score, 0)
        if (expected.join("\n") !== actual.join("\n")) {
            api.log.info("[testflow] " + what + ", part " + parts[i].title + " annotations: ["
                         + ScoreContent.allAnnotations(parts[i].score).join("] [") + "]")
            api.log.info("[testflow] " + what + ", score annotations: ["
                         + ScoreContent.allAnnotations(engraving.curScore).join("] [") + "]")
        }
        ScoreContent.expectSameLines(expected, actual, what + ", part " + parts[i].title)
    }
}

function snapshotAll()
{
    var result = { score: ScoreContent.contentLines(engraving.curScore), parts: [] }
    var parts = Parts.list()
    for (var i = 0; i < parts.length; ++i) {
        result.parts.push({ title: parts[i].title, lines: ScoreContent.contentLines(parts[i].score) })
    }
    return result
}

var testCase = {
    name: "TC12: Score and Parts, Save and Reopen",
    description: "Make a Flute + Oboe score, open parts, add notes with ties, a triplet, articulations, a slur, a hairpin, a dynamic, lyrics, staff text and a tempo change. Check the parts follow the score, then save, close, reopen and check the score and parts are unchanged",
    steps: [
        {name: "Close score (if opened) and go to home to start", func: function() {
            Project.closeIfOpened()
            Home.goToHome()
        }},
        {name: "Open New Score Dialog", func: function() {
            NewScore.openNewScoreDialog()
        }},
        {name: "Select Flute and Oboe", func: function() {
            NewScore.selectTab("instruments")
            NewScore.chooseInstrument("Woodwinds", "Flute")
            api.testflow.seeChanges()
            //! NOTE Woodwinds is already selected, so pick Oboe from the same list
            NewScore.selectInstrument("Oboe")
            //! NOTE Adding an instrument is not instant, give it time before pressing Done
            api.testflow.sleep(1000)
        }},
        {name: "Create score with Flute and Oboe", func: function() {
            NewScore.done()
            if (!waitForProject(5000)) {
                api.testflow.fatal("no score is opened")
            }
            var ids = []
            for (var i = 0; i < engraving.curScore.parts.length; ++i) {
                ids.push(engraving.curScore.parts[i].instrumentId)
            }
            if (ids.length !== 2) {
                api.testflow.fatal("create score: expected 2 instruments, got [" + ids.join(", ") + "]")
            }
            api.log.info("[testflow] create score: OK (" + ids.join(", ") + ")")
        }},
        {name: "Open all parts", func: function() {
            Parts.openAll()
            Parts.goToMainScore()
            //! NOTE Parts may take a moment to be created
            var parts = Parts.list()
            for (var waited = 0; parts.length < 2 && waited < 5000; waited += 200) {
                api.testflow.sleep(200)
                parts = Parts.list()
            }
            if (parts.length !== 2) {
                var titles = parts.map(function(p) { return p.title })
                api.testflow.fatal("open parts: expected 2 parts, got " + parts.length + " [" + titles.join(", ")
                                   + "]; current score has " + engraving.curScore.parts.length + " instruments")
            }
            api.log.info("[testflow] open parts: OK (" + parts[0].title + ", " + parts[1].title + ")")
        }},
        {name: "Put flute notes: articulations, a tie, a triplet", func: function() {
            selectFluteNote(0) // the first rest
            NoteInput.chooseDefaultMode()

            NoteInput.chooseNoteDuration(Commands.SET_DURATION_QUARTER_COMMAND)
            NoteInput.toggleArticulation(Commands.TOGGLE_STACCATO_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            NoteInput.toggleArticulation(Commands.TOGGLE_STACCATO_COMMAND) // off
            NoteInput.toggleArticulation(Commands.TOGGLE_TENUTO_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_D_COMMAND)
            NoteInput.toggleArticulation(Commands.TOGGLE_TENUTO_COMMAND) // off
            NoteInput.putNote(Commands.ENTER_NOTE_E_COMMAND)
            NoteInput.putTie()

            NoteInput.chooseNoteDuration(Commands.SET_DURATION_EIGHTH_COMMAND)
            api.dispatcher.dispatch(ADD_TRIPLET_COMMAND)
            api.testflow.sleep(100)
            NoteInput.putNote(Commands.ENTER_NOTE_F_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_G_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_A_COMMAND)

            NoteInput.chooseNoteDuration(Commands.SET_DURATION_QUARTER_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_B_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_C_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_D_COMMAND)

            NoteInput.chooseNoteDuration(Commands.SET_DURATION_HALF_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_E_COMMAND)
            NoteInput.putNote(Commands.ENTER_NOTE_F_COMMAND)

            NoteInput.turnoffDefaultMode()
            api.testflow.seeChanges()

            for (var i = 0; i < EXPECTED_NOTES.length; ++i) {
                var e = EXPECTED_NOTES[i]
                var el = ScoreContent.chordRestAt(0, e.tick)
                if (!el || el.type !== engraving.Element.CHORD) {
                    api.testflow.fatal("put notes: no chord at tick " + e.tick)
                }
                var note = el.notes[0]
                var ok = el.actualDuration.ticks === e.len && note.pitch % 12 === e.pc
                         && !!note.tieForward === e.tie && !!el.tuplet === e.tuplet
                if (!ok) {
                    api.testflow.fatal("put notes: chord at tick " + e.tick + " is wrong (length " + el.actualDuration.ticks
                                       + ", pitch " + note.pitch + ", tied " + !!note.tieForward + ", tuplet " + !!el.tuplet + ")")
                }
            }
            if (ScoreContent.chordRestAt(0, 0).articulations.length !== 1
                || ScoreContent.chordRestAt(0, 480).articulations.length !== 1
                || ScoreContent.chordRestAt(0, 960).articulations.length !== 0) {
                api.testflow.fatal("put notes: expected one articulation on each of the first 2 notes only")
            }
            api.log.info("[testflow] put notes: OK (" + EXPECTED_NOTES.length + " notes, tie, triplet, articulations)")
        }},
        {name: "Add a slur over the triplet", func: function() {
            selectFluteRange(1920, 2400)
            api.dispatcher.dispatch(ADD_SLUR_COMMAND)
            api.testflow.seeChanges()
            endTextEdit() // adding a slur may select it for editing
            ScoreContent.expectLine(ScoreContent.contentLines(engraving.curScore), "spanner: Slur staff 0 from @1920 to @2240", "add slur")
        }},
        {name: "Add a hairpin over B C D", func: function() {
            selectFluteRange(2400, 3840)
            api.dispatcher.dispatch(ADD_HAIRPIN_COMMAND)
            api.testflow.seeChanges()
            endTextEdit()
            ScoreContent.expectLine(ScoreContent.contentLines(engraving.curScore), "spanner: HairPin type 0 staff 0 from @2400", "add hairpin (crescendo)")
        }},
        {name: "Add a dynamic (f) on the first note", func: function() {
            selectFluteNote(0)
            startTextEdit(ADD_DYNAMIC_COMMAND)
            api.keyboard.text("f")
            endTextEdit()
            ScoreContent.expectLine(ScoreContent.contentLines(engraving.curScore), "staff 0: Dynamic @0", "add dynamic")
        }},
        {name: "Add lyrics on the first 3 notes", func: function() {
            selectFluteNote(0)
            startTextEdit(ADD_LYRICS_COMMAND)
            api.keyboard.text("la")
            api.keyboard.key(" ")
            api.testflow.sleep(100)
            api.keyboard.text("li")
            api.keyboard.key(" ")
            api.testflow.sleep(100)
            api.keyboard.text("lo")
            endTextEdit()
            var lines = ScoreContent.contentLines(engraving.curScore)
            ScoreContent.expectLine(lines, "lyrics la", "add lyrics (la)")
            ScoreContent.expectLine(lines, "lyrics li", "add lyrics (li)")
            ScoreContent.expectLine(lines, "lyrics lo", "add lyrics (lo)")
        }},
        {name: "Add staff text on the first half note", func: function() {
            selectFluteNote(3840)
            startTextEdit(ADD_STAFF_TEXT_COMMAND)
            api.keyboard.text("con brio")
            endTextEdit()
            ScoreContent.expectLine(ScoreContent.contentLines(engraving.curScore), "staff 0: StaffText @3840 \"con brio\"", "add staff text")
        }},
        {name: "Add a tempo change at the triplet", func: function() {
            selectFluteNote(1920)
            startTextEdit(ADD_TEMPO_COMMAND)
            endTextEdit() // keep the default tempo text
            ScoreContent.expectLine(ScoreContent.contentLines(engraving.curScore), "Tempo @1920", "add tempo")
        }},
        {name: "Check the parts follow the score", func: function() {
            checkPartsMatchScore("before save")
        }},
        {name: "Remember what is saved", func: function() {
            var state = snapshotAll()
            savedScore = state.score
            savedParts = state.parts
            api.log.info("[testflow] saving: score " + savedScore.length + " lines, " + savedParts.length + " parts")
        }},
        {name: "Save", func: function() {
            api.testflow.saveProject(FILE_NAME)
            api.testflow.seeChanges()
        }},
        {name: "Close", func: function() {
            api.dispatcher.dispatch(Commands.PROJECT_CLOSE_COMMAND)
            api.testflow.seeChanges()
            //! NOTE If the score wasn't saved, a "save changes?" question would keep it open
            if (Project.isOpened()) {
                api.testflow.fatal("the score is still open after closing")
            }
        }},
        {name: "Reopen from Recent scores", func: function() {
            Home.goToHome()
            api.testflow.seeChanges()
            Home.openLastProject()
            if (!waitForProject(10000)) {
                api.testflow.fatal("the score did not reopen")
            }
            api.testflow.seeChanges()
            Parts.goToMainScore()
        }},
        {name: "Check the reopened score and parts", func: function() {
            var state = snapshotAll()
            ScoreContent.expectSameLines(savedScore, state.score, "after reopen, score")
            if (state.parts.length !== savedParts.length) {
                api.testflow.fatal("after reopen: expected " + savedParts.length + " parts, got " + state.parts.length)
            }
            for (var i = 0; i < savedParts.length; ++i) {
                if (state.parts[i].title !== savedParts[i].title) {
                    api.testflow.fatal("after reopen: part " + (i + 1) + " is [" + state.parts[i].title + "], expected [" + savedParts[i].title + "]")
                }
                ScoreContent.expectSameLines(savedParts[i].lines, state.parts[i].lines, "after reopen, part " + savedParts[i].title)
            }
            checkPartsMatchScore("after reopen")
        }}
    ]
};

function main()
{
    api.testflow.setInterval(500)
    api.testflow.runTestCase(testCase)
}
