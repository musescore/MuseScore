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

//! NOTE How long to let a dispatched command run before checking its result:
//! dispatch() only queues the command
const COMMAND_WAIT_MS = 500

//! NOTE The measure at `index` (0 = the first) of `score` (by default the current score)
function measureAt(index, score)
{
    var measure = (score || engraving.curScore).firstMeasure
    for (var i = 0; i < index && measure; ++i) {
        measure = measure.nextMeasure
    }
    if (!measure) {
        api.testflow.fatal("no measure " + (index + 1))
    }
    return measure
}

//! NOTE A note value as written: 4 for a quarter, 8. for a dotted eighth
function noteValue(duration)
{
    var numerator = duration.numerator
    var denominator = duration.denominator
    var dots = 0
    while (numerator > 1 && (numerator + 1) % 2 === 0) {
        numerator = (numerator - 1) / 2
        denominator /= 2
        ++dots
    }
    var dotText = ""
    for (var i = 0; i < dots; ++i) {
        dotText += "."
    }
    return denominator + dotText
}

//! NOTE The tuplets that `chordRest` is in, the outermost first
function tupletsOf(chordRest)
{
    var tuplets = []
    for (var t = chordRest.tuplet; t; t = t.tuplet) {
        tuplets.unshift(t)
    }
    return tuplets
}

//! NOTE The rhythm of a track in a measure, written as in the engine's tuplet tests, e.g.
//! "3:2[8~ 8 r8] 8. 4": note values (r = rest, . = dot, ~ = tie, R = full-measure rest),
//! tuplets as actual:normal[...], nested ones inside their outer tuplet.
//! `score` is the current score by default (a part's score to read a part)
function rhythm(measureIndex, track, score)
{
    track = track || 0
    var result = ""
    var open = []
    for (var segment = measureAt(measureIndex, score).firstSegment; segment; segment = segment.nextInMeasure) {
        var cr = segment.elementAt(track)
        if (!cr || (cr.type !== engraving.Element.CHORD && cr.type !== engraving.Element.REST)) {
            continue
        }

        var tuplets = tupletsOf(cr)
        var common = 0
        while (common < open.length && common < tuplets.length && open[common].is(tuplets[common])) {
            ++common
        }
        while (open.length > common) {
            open.pop()
            result += "]"
        }
        for (var i = common; i < tuplets.length; ++i) {
            result += " " + tuplets[i].actualNotes + ":" + tuplets[i].normalNotes + "["
            open.push(tuplets[i])
        }

        result += (result.length === 0 || result[result.length - 1] === "[") ? "" : " "
        if (cr.type === engraving.Element.REST && cr.isFullMeasureRest) {
            result += "R"
            continue
        }
        var isChord = cr.type === engraving.Element.CHORD
        result += isChord ? "" : "r"
        result += noteValue(cr.duration)
        result += isChord && cr.notes[cr.notes.length - 1].tieForward ? "~" : ""
    }
    while (open.length > 0) {
        open.pop()
        result += "]"
    }
    return result.trim()
}

//! NOTE Fails the test case unless the measure's rhythm is `expected`
function expectRhythm(measureIndex, expected, what, track)
{
    var actual = rhythm(measureIndex, track)
    if (actual !== expected) {
        api.testflow.fatal(what + ": measure " + (measureIndex + 1) + " expected [" + expected + "], got [" + actual + "]")
    }
    api.log.info("[testflow] " + what + ": OK [" + actual + "]")
}

//! NOTE The innermost tuplet of the chord or rest at the start of a measure (+ `offsetTicks`), or null
function tupletAt(measureIndex, offsetTicks, track)
{
    var measure = measureAt(measureIndex)
    var tick = measure.tick.ticks + (offsetTicks || 0)
    for (var segment = measure.firstSegment; segment; segment = segment.nextInMeasure) {
        var cr = segment.elementAt(track || 0)
        if (cr && segment.tick === tick && (cr.type === engraving.Element.CHORD || cr.type === engraving.Element.REST)) {
            return cr.tuplet
        }
    }
    return null
}

//! NOTE Dispatches a command and lets it run before the result is checked
function send(command)
{
    api.dispatcher.dispatch(command)
    api.testflow.sleep(COMMAND_WAIT_MS)
}

module.exports = {
    measureAt: measureAt,
    rhythm: rhythm,
    expectRhythm: expectRhythm,
    tupletAt: tupletAt,
    send: send
}
