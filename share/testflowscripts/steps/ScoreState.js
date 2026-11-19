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

//! NOTE A note as stored in a snapshot: where it starts, how long it is and its pitch
function noteKey(tick, duration, pitch)
{
    return tick + ":" + duration + ":" + pitch
}

//! NOTE All notes on one track, in score order: [{tick, duration, pitch}]
//! tick and duration are in ticks (a quarter note is 480).
function notesOnTrack(track)
{
    var score = engraving.curScore
    var result = []
    if (!score) {
        return result
    }

    var cursor = score.newCursor()
    cursor.track = track
    cursor.rewind(0) // SCORE_START
    while (cursor.segment) {
        var el = cursor.element
        if (el && el.type === engraving.Element.CHORD) {
            var duration = el.actualDuration.ticks
            var notes = el.notes
            for (var n = 0; n < notes.length; ++n) {
                result.push({ tick: cursor.tick, duration: duration, pitch: notes[n].pitch })
            }
        }
        cursor.next()
    }
    return result
}

//! NOTE A compact description of the score content, used to check that an action
//! (or its undo/redo) left the score in the expected state.
//! Contains the measure count, the track count, and for every track the
//! position (tick), duration and pitch of each note.
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
        content: []
    }

    for (var t = 0; t < score.ntracks; ++t) {
        var keys = notesOnTrack(t).map(function(n) { return noteKey(n.tick, n.duration, n.pitch) })
        state.notes += keys.length
        state.content.push(keys.join(","))
    }

    return state
}

//! NOTE Describes the first difference between two snapshots, for error messages
function firstDifference(expected, actual)
{
    if (!expected || !actual) {
        return ""
    }
    for (var t = 0; t < expected.content.length; ++t) {
        var e = expected.content[t] || ""
        var a = (actual.content && actual.content[t]) || ""
        if (e !== a) {
            return "; track " + t + " expected notes (tick:duration:pitch) [" + e + "], got [" + a + "]"
        }
    }
    return ""
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
        api.testflow.fatal(what + ": expected [" + describe(expected) + "], got [" + describe(actual) + "]"
                           + firstDifference(expected, actual))
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

//! NOTE Fails the test case if the notes on `track` are not exactly `expected`
//! ([{tick, duration, pitch}], in score order)
function expectTrackNotes(track, expected, what)
{
    var actual = notesOnTrack(track)
    var e = expected.map(function(n) { return noteKey(n.tick, n.duration, n.pitch) }).join(",")
    var a = actual.map(function(n) { return noteKey(n.tick, n.duration, n.pitch) }).join(",")
    if (e !== a) {
        api.testflow.fatal(what + ": track " + track + " expected notes (tick:duration:pitch) [" + e + "], got [" + a + "]")
    }
    api.log.info("[testflow] " + what + ": OK (" + actual.length + " notes at the expected positions)")
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
    selectMeasures: selectMeasures,
    notesOnTrack: notesOnTrack,
    snapshot: snapshot,
    describe: describe,
    equals: equals,
    expectEquals: expectEquals,
    expectMeasures: expectMeasures,
    expectNotes: expectNotes,
    expectTrackNotes: expectTrackNotes
}
