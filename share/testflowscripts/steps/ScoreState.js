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

var engraving = require("MuseApi.Engraving")

//! NOTE A compact description of the score content, used to check that an action
//! (or its undo/redo) left the score in the expected state.
//! Contains the measure count, the track count and the pitches of all notes per track.
function snapshot()
{
    var score = engraving.curScore
    if (!score) {
        return null
    }

    var state = {
        measures: score.nmeasures,
        tracks: score.ntracks,
        notes: 0,
        pitches: []
    }

    var cursor = score.newCursor()
    for (var t = 0; t < score.ntracks; ++t) {
        cursor.track = t
        cursor.rewind(0) // SCORE_START
        var pitches = []
        while (cursor.segment) {
            var el = cursor.element
            if (el && el.type === engraving.Element.CHORD) {
                var notes = el.notes
                for (var n = 0; n < notes.length; ++n) {
                    pitches.push(notes[n].pitch)
                }
            }
            cursor.next()
        }
        state.notes += pitches.length
        state.pitches.push(pitches.join(","))
    }

    return state
}

function describe(state)
{
    if (!state) {
        return "<no score>"
    }
    return "measures: " + state.measures + ", tracks: " + state.tracks + ", notes: " + state.notes
}

function equals(a, b)
{
    return JSON.stringify(a) === JSON.stringify(b)
}

//! NOTE Fails the test case if the current score is not equal to the expected state
function expectEquals(expected, what)
{
    var actual = snapshot()
    if (!equals(actual, expected)) {
        api.testflow.fatal(what + ": expected [" + describe(expected) + "], got [" + describe(actual) + "]")
    }
    api.log.info("[testflow] " + what + ": OK [" + describe(actual) + "]")
}

function expectMeasures(count, what)
{
    var actual = engraving.curScore ? engraving.curScore.nmeasures : -1
    if (actual !== count) {
        api.testflow.fatal(what + ": expected " + count + " measures, got " + actual)
    }
    api.log.info("[testflow] " + what + ": OK (" + actual + " measures)")
}

function expectNotes(count, what)
{
    var actual = snapshot().notes
    if (actual !== count) {
        api.testflow.fatal(what + ": expected " + count + " notes, got " + actual)
    }
    api.log.info("[testflow] " + what + ": OK (" + actual + " notes)")
}


// Select `count` whole measures on all staves as a range, starting at the
// measure with index `startIndex` (0 = first measure).
// The range end is given as the start of the next measure, because the
// selection end is exclusive. That means the range can't include the last
// measure of the score.
function selectMeasures(startIndex, count)
{
    var score = engraving.curScore
    var first = score.firstMeasure
    for (var i = 0; i < startIndex && first; ++i) {
        first = first.nextMeasure
    }
    var after = first
    for (var j = 0; j < count && after; ++j) {
        after = after.nextMeasure
    }
    if (!first || !after) {
        api.testflow.fatal("selectMeasures: range " + startIndex + "+" + count + " must end before the last measure")
    }
    var startTick = first.tick.ticks
    var endTick = after.tick.ticks
    if (!score.selection.selectRange(startTick, endTick, 0, score.nstaves)) {
        api.testflow.fatal("selectMeasures: could not select ticks " + startTick + "-" + endTick)
    }
    api.log.info("[testflow] selected " + count + " measures from index " + startIndex + " (ticks " + startTick + "-" + endTick + ")")
}

module.exports = {
    selectMeasures,
    snapshot: snapshot,
    describe: describe,
    equals: equals,
    expectEquals: expectEquals,
    expectMeasures: expectMeasures,
    expectNotes: expectNotes
}
