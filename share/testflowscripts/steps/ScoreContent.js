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

//! NOTE Annotations that belong to the whole system: every part shows them
const SYSTEM_ANNOTATIONS = ["Tempo", "SystemText", "RehearsalMark"]

function ticksOf(el)
{
    return (el && el.fraction) ? el.fraction.ticks : -1
}

function textOf(el)
{
    if (el.plainText !== undefined && el.plainText !== null) {
        return String(el.plainText)
    }
    if (el.text !== undefined && el.text !== null) {
        return String(el.text)
    }
    return ""
}

function isSystemAnnotation(el)
{
    return SYSTEM_ANNOTATIONS.indexOf(el.name) !== -1
}

function chordRestLine(el, tick)
{
    var isChord = el.type === engraving.Element.CHORD
    var line = (isChord ? "chord" : "rest") + " @" + tick + " len " + el.actualDuration.ticks
    if (el.tuplet) {
        line += " tuplet"
    }
    if (isChord) {
        var pitches = []
        for (var n = 0; n < el.notes.length; ++n) {
            var note = el.notes[n]
            pitches.push(note.pitch + (note.tieForward ? "~" : ""))
        }
        line += " notes " + pitches.join("+")

        var arts = []
        for (var a = 0; a < el.articulations.length; ++a) {
            arts.push(String(el.articulations[a].symbol))
        }
        if (arts.length > 0) {
            line += " art " + arts.join("+")
        }
    }
    var lyrics = []
    for (var l = 0; l < el.lyrics.length; ++l) {
        lyrics.push(textOf(el.lyrics[l]))
    }
    if (lyrics.length > 0) {
        line += " lyrics " + lyrics.join("+")
    }
    return line
}

//! NOTE A line-by-line description of a score's content, for comparing a score
//! (or a part) before and after save/reopen. Covers:
//! - measures, tracks and instruments
//! - every chord and rest per track: position, length, tuplet, pitches, ties,
//!   articulations and lyrics
//! - annotations: dynamics, staff text, and system items such as tempo
//! - spanners (slurs, hairpins and other lines) with their start and end
//!
//! With `onlyStaff`, describes just that staff, numbered as staff 0, plus the
//! system items. That's what a part made from that staff should contain,
//! so a part can be compared to its staff in the main score.
function contentLines(score, onlyStaff)
{
    var lines = []
    if (!score) {
        return lines
    }

    var single = onlyStaff !== undefined
    if (!single) {
        var ids = []
        for (var p = 0; p < score.parts.length; ++p) {
            ids.push(score.parts[p].instrumentId)
        }
        lines.push("measures " + score.nmeasures + ", tracks " + score.ntracks + ", instruments " + ids.join(","))
    } else {
        lines.push("measures " + score.nmeasures)
    }

    var firstTrack = single ? onlyStaff * 4 : 0
    var endTrack = single ? firstTrack + 4 : score.ntracks
    var cursor = score.newCursor()
    var systemLines = []

    for (var t = firstTrack; t < endTrack; ++t) {
        cursor.track = t
        cursor.rewind(0) // SCORE_START
        while (cursor.segment) {
            var el = cursor.element
            if (el && (el.type === engraving.Element.CHORD || el.type === engraving.Element.REST)) {
                lines.push("track " + (t - firstTrack) + ": " + chordRestLine(el, cursor.tick))
            }

            cursor.next()
        }
    }

    //! NOTE Annotations (dynamics, staff text, tempo...), listed by staff
    var anns = annotationsOf(score)
    for (var i = 0; i < anns.length; ++i) {
        var ann = anns[i].ann
        var line = ann.name + " @" + anns[i].tick + " \"" + textOf(ann) + "\""
        if (single && isSystemAnnotation(ann)) {
            //! NOTE System items are kept on the top staff of the score, but appear in every part
            if (ann.staffIdx === 0) {
                systemLines.push("system: " + line)
            }
            continue
        }
        if (!single || ann.staffIdx === onlyStaff) {
            lines.push("staff " + (single ? 0 : ann.staffIdx) + ": " + line)
        }
    }
    systemLines.sort()

    var spanners = []
    for (var s = 0; s < score.spanners.length; ++s) {
        var sp = score.spanners[s]
        if (single && sp.staffIdx !== onlyStaff) {
            continue
        }
        spanners.push("spanner: " + sp.name + " staff " + (single ? 0 : sp.staffIdx)
                      + " from @" + ticksOf(sp.startElement) + " to @" + ticksOf(sp.endElement))
    }
    spanners.sort()

    return lines.concat(systemLines, spanners)
}

//! NOTE All annotations of the score with their ticks, in score order.
//! In a part made of one long multi-measure rest, MuseScore can keep items like tempo marks
//! on the multi-measure rest instead of the real measure, so both are read; each item is listed once.
function annotationsOf(score)
{
    var result = []
    var seen = {}
    var add = function(seg) {
        var anns = seg.annotations
        for (var i = 0; i < anns.length; ++i) {
            var key = anns[i].name + "|" + anns[i].staffIdx + "|" + seg.tick + "|" + textOf(anns[i])
            if (!seen[key]) {
                seen[key] = true
                result.push({ ann: anns[i], tick: seg.tick })
            }
        }
    }
    for (var m = score.firstMeasure; m; m = m.nextMeasure) {
        for (var seg = m.firstSegment; seg; seg = seg.nextInMeasure) {
            add(seg)
        }
    }
    for (var mm = score.firstMeasureMM; mm; mm = mm.nextMeasureMM) {
        for (var mseg = mm.firstSegment; mseg; mseg = mseg.nextInMeasure) {
            add(mseg)
        }
    }
    result.sort(function(a, b) { return a.tick - b.tick })
    return result
}

//! NOTE Every annotation in the score (any staff, any segment), for diagnostics
function allAnnotations(score)
{
    var result = ["measures " + score.nmeasures]
    for (var mm = score.firstMeasureMM; mm; mm = mm.nextMeasureMM) {
        for (var mseg = mm.firstSegment; mseg; mseg = mseg.nextInMeasure) {
            var manns = mseg.annotations
            for (var j = 0; j < manns.length; ++j) {
                result.push("(mm) " + manns[j].name + " staff " + manns[j].staffIdx + " @" + mseg.tick)
            }
        }
    }
    for (var m = score.firstMeasure; m; m = m.nextMeasure) {
        for (var seg = m.firstSegment; seg; seg = seg.nextInMeasure) {
            var anns = seg.annotations
            for (var i = 0; i < anns.length; ++i) {
                result.push(anns[i].name + " staff " + anns[i].staffIdx + " @" + seg.tick + " \"" + textOf(anns[i]) + "\"")
            }
        }
    }
    return result
}

//! NOTE Fails with the first differing line if the two descriptions differ
function expectSameLines(expected, actual, what)
{
    var count = Math.max(expected.length, actual.length)
    for (var i = 0; i < count; ++i) {
        if (expected[i] !== actual[i]) {
            api.testflow.fatal(what + ": line " + (i + 1) + " differs: expected [" + (expected[i] || "<nothing>")
                               + "], got [" + (actual[i] || "<nothing>") + "]")
        }
    }
    api.log.info("[testflow] " + what + ": OK (" + actual.length + " lines match)")
}

//! NOTE Fails unless a line containing `text` is in `lines`
function expectLine(lines, text, what)
{
    for (var i = 0; i < lines.length; ++i) {
        if (lines[i].indexOf(text) !== -1) {
            api.log.info("[testflow] " + what + ": OK [" + lines[i] + "]")
            return lines[i]
        }
    }
    //! NOTE List the lines of the same kind (e.g. all "spanner:" lines) to show what is there instead
    var kind = text.split(":")[0] + ":"
    var similar = []
    for (var j = 0; j < lines.length; ++j) {
        if (lines[j].indexOf(kind) === 0) {
            similar.push(lines[j])
        }
    }
    api.testflow.fatal(what + ": no line with [" + text + "]; " + kind + " lines: ["
                       + (similar.length ? similar.join("] [") : "none") + "]")
    return ""
}

//! NOTE The chord or rest at `tick` on `track` of the current score, or null
function chordRestAt(track, tick)
{
    var score = engraving.curScore
    var cursor = score.newCursor()
    cursor.track = track
    cursor.rewind(0)
    while (cursor.segment) {
        if (cursor.tick === tick && cursor.element) {
            return cursor.element
        }
        if (cursor.tick > tick) {
            break
        }
        cursor.next()
    }
    return null
}

//! NOTE Selects the (top) note of the chord at `tick` on `track`, or the rest there
function selectAt(track, tick)
{
    var el = chordRestAt(track, tick)
    if (!el) {
        api.testflow.fatal("nothing to select at tick " + tick + " on track " + track)
    }
    var target = el.type === engraving.Element.CHORD ? el.notes[el.notes.length - 1] : el
    if (!engraving.curScore.selection.select(target)) {
        api.testflow.fatal("could not select the element at tick " + tick)
    }
}

module.exports = {
    contentLines: contentLines,
    expectSameLines: expectSameLines,
    allAnnotations: allAnnotations,
    expectLine: expectLine,
    chordRestAt: chordRestAt,
    selectAt: selectAt
}
