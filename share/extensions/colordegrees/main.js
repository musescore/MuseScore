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

// Degree colors: I, ii, iii, IV, V, vi, vii°
const DEGREE_COLORS = [
    "#62bc47",  // 1 - Tonic (I),        Good Green
    "#1560bd",  // 2 - Supertonic (ii),  Calm Blue
    "#b32d00",  // 3 - Mediant (iii),    Aggressive Red-Orange
    "#fff700",  // 4 - Subdominant (IV), Cheerful Yellow
    "#5db0f0",  // 5 - Dominant (V),     Adventurous Blue
    "#ff4500",  // 6 - Submediant (vi),  Sad Reddish Violet
    "#ffa500"   // 7 - Leading tone (vii°), Orange (between yellow and red-orange)
];
const BLACK = "#000000";
const TRIAD_GREY = "#808080";  // Grey for 3rd and 5th (triad members, not root)

// keysig (-7 to +7) -> tonic pitch class (0-11)
const TONIC_CHROMA = {
    "-7": 11, "-6": 6, "-5": 1, "-4": 8, "-3": 3, "-2": 10, "-1": 5,
    "0": 0, "1": 7, "2": 2, "3": 9, "4": 4, "5": 11, "6": 6, "7": 1
};
const MAJOR_SCALE = [0, 2, 4, 5, 7, 9, 11]; // semitones from tonic

function pitchToDegree(pitch, keysig) {
    const chroma = pitch % 12;
    const tonic = TONIC_CHROMA[String(keysig)] !== undefined ? TONIC_CHROMA[String(keysig)] : 0;
    const semitonesFromTonic = (chroma - tonic + 12) % 12;
    const degreeIdx = MAJOR_SCALE.indexOf(semitonesFromTonic);
    return degreeIdx >= 0 ? degreeIdx + 1 : 0;
}

function colorNote(note, color) {
    note.color = color;
    if (note.accidental) note.accidental.color = color;
    if (note.dots) {
        for (var d = 0; d < note.dots.length; d++) {
            if (note.dots[d]) note.dots[d].color = color;
        }
    }
}

//--- Weighted Third-Stacking Algorithm (from algorithm.txt) ---

// Phase 1: Score a note by duration, beat strength, bass position, repetition
function scoreNote(noteInfo, measureTicks, measureStartTick, lowestPitchInBar, chromaCounts) {
    var score = 0;
    // Duration: longer = more likely chord tone
    var durTicks = noteInfo.durationTicks;
    score += durTicks;  // raw ticks as weight (half note >> 16th)

    // Beat strength: strong beats (1 and 3 in 4/4) = chord tones
    var tickInMeasure = noteInfo.tick - measureStartTick;
    var quarterTicks = measureTicks / 4;  // assume 4 beats
    var beat = Math.floor(tickInMeasure / quarterTicks);
    if (beat === 0 || beat === 2) score += 100;  // strong beats

    // Bass: lowest note in bar carries harmonic weight
    if (noteInfo.pitch === lowestPitchInBar) score += 150;

    // Repetition: note appearing multiple times in arpeggio
    var chroma = noteInfo.pitch % 12;
    score += (chromaCounts[chroma] || 0) * 20;

    return score;
}

// Find chord root by scoring each note as a potential root based on harmonic intervals
function findChordRoot(pitchClasses) {
    if (!pitchClasses || pitchClasses.length === 0) return -1;
    if (pitchClasses.length === 1) return pitchClasses[0];

    // Remove duplicates to ensure clean scoring
    var pcs = [];
    for (var k = 0; k < pitchClasses.length; k++) {
        if (pcs.indexOf(pitchClasses[k]) === -1) pcs.push(pitchClasses[k]);
    }

    var bestRoot = -1;
    var highestScore = -1;

    // Treat every note in the array as a potential root and score it
    for (var i = 0; i < pcs.length; i++) {
        var potentialRoot = pcs[i];
        var score = 0;

        for (var j = 0; j < pcs.length; j++) {
            if (i === j) continue;

            // Calculate interval UPWARDS from the potential root
            var interval = (pcs[j] - potentialRoot + 12) % 12;

            // Score based on harmonic importance to the root
            if (interval === 7) {
                score += 3; // Perfect 5th is the strongest root indicator
            } else if (interval === 4 || interval === 3) {
                score += 2; // Major or Minor 3rd is the next strongest
            } else if (interval === 10 || interval === 11) {
                score += 1; // Minor or Major 7th provides good context
            }
        }

        // If this note has more harmonic support, it becomes the new best root
        if (score > highestScore) {
            highestScore = score;
            bestRoot = potentialRoot;
        }
    }

    // Tie-breaker/Fallback: If no intervals matched (score is 0), just return the bass note (first note)
    return highestScore > 0 ? bestRoot : pcs[0];
}

function log(msg) { try { if (api && api.log) api.log.info(msg); } catch (e) {} }

function processMeasureNotes(noteInfos, measureTicks, measureStartTick, keysig) {
    if (noteInfos.length === 0) return;
    log("colordegrees: processMeasureNotes n=" + noteInfos.length + " tick=" + measureStartTick);

    var lowestPitchInBar = 9999;
    var chromaCounts = {};
    for (var i = 0; i < noteInfos.length; i++) {
        var chroma = noteInfos[i].pitch % 12;
        chromaCounts[chroma] = (chromaCounts[chroma] || 0) + 1;
        if (noteInfos[i].pitch < lowestPitchInBar) lowestPitchInBar = noteInfos[i].pitch;
    }

    // Phase 1: Score each note
    for (var i = 0; i < noteInfos.length; i++) {
        noteInfos[i].score = scoreNote(noteInfos[i], measureTicks, measureStartTick, lowestPitchInBar, chromaCounts);
    }

    // Phase 2: Top 3-4 by score, collapse to pitch classes
    noteInfos.sort(function(a, b) { return b.score - a.score; });
    var pitchClasses = [];
    var seen = {};
    for (var i = 0; i < noteInfos.length; i++) {
        var chroma = noteInfos[i].pitch % 12;
        if (!seen[chroma]) {
            seen[chroma] = true;
            pitchClasses.push(chroma);
            // Stop once we have gathered the 4 strongest unique pitch classes
            if (pitchClasses.length >= 4) {
                break;
            }
        }
    }

    if (pitchClasses.length === 0) return;

    // Phase 3 & 4: Find root
    var rootChroma = findChordRoot(pitchClasses);
    if (rootChroma < 0) return;

    // Map root chroma to scale degree (1-7) in current key
    var tonic = TONIC_CHROMA[String(keysig)] !== undefined ? TONIC_CHROMA[String(keysig)] : 0;
    var semitonesFromTonic = (rootChroma - tonic + 12) % 12;
    var degreeIdx = MAJOR_SCALE.indexOf(semitonesFromTonic);
    var degree = degreeIdx >= 0 ? degreeIdx + 1 : 0;
    var color = (degree >= 1 && degree <= 7) ? DEGREE_COLORS[degree - 1] : BLACK;

    // Root = degree color; triad = grey; other diatonic = their degree color; accidental = black
    log("colordegrees: coloring " + noteInfos.length + " notes degree=" + degree);
    for (var i = 0; i < noteInfos.length; i++) {
        var noteObj = noteInfos[i].note;
        if (noteObj.accidental) {
            colorNote(noteObj, BLACK);  // Accidental notes (chromatic) = black
            continue;
        }
        var chroma = noteInfos[i].pitch % 12;
        var semitonesFromRoot = (chroma - rootChroma + 12) % 12;
        var noteColor;
        if (semitonesFromRoot === 0) {
            noteColor = color;  // Root: chord degree color
        } else if (semitonesFromRoot === 3 || semitonesFromRoot === 4 || semitonesFromRoot === 7) {
            noteColor = TRIAD_GREY;  // 3rd (m3/M3) or 5th (P5): grey
        } else {
            // Other notes: color by their scale degree in the key
            var noteDegree = pitchToDegree(noteInfos[i].pitch, keysig);
            noteColor = (noteDegree >= 1 && noteDegree <= 7) ? DEGREE_COLORS[noteDegree - 1] : BLACK;
        }
        colorNote(noteObj, noteColor);
    }
}

// Toggle single note: color by degree, or remove if already has that degree color
function toggleSingleNote(note) {
    if (!note || note.pitch === undefined) return;
    curScore.startCmd();
    var keysig = curScore.keysig !== undefined ? curScore.keysig : 0;
    if (note.accidental) {
        colorNote(note, BLACK);
    } else {
        var noteDegree = pitchToDegree(note.pitch, keysig);
        var targetColor = (noteDegree >= 1 && noteDegree <= 7) ? DEGREE_COLORS[noteDegree - 1] : BLACK;
        // Toggle: if note already has this color, remove it; otherwise apply it
        var curColor = note.color ? String(note.color) : "";
        var hasTargetColor = curColor.indexOf(targetColor) >= 0 || curColor === targetColor;
        var applyColor = hasTargetColor ? BLACK : targetColor;
        log("colordegrees: single note pitch=" + note.pitch + " degree=" + noteDegree + " target=" + targetColor + " cur=" + curColor + " apply=" + applyColor);
        colorNote(note, applyColor);
    }
    curScore.endCmd();
}

function main() {
    log("colordegrees: main start");
    if (!curScore) return;

    var sel = curScore.selection;

    // Single note selected: toggle its color (degree color or remove)
    if (sel && sel.elements && sel.elements.length === 1) {
        var el = sel.elements[0];
        var note = null;
        if (el && el.pitch !== undefined) {
            note = el;
        } else if (el && el.notes && el.notes.length === 1) {
            note = el.notes[0];  // Chord with single note
        }
        if (note) {
            toggleSingleNote(note);
            return;
        }
    }

    var keysig = curScore.keysig !== undefined ? curScore.keysig : 0;
    var ntracks = curScore.ntracks || 16;
    var SEGMENT_CHORDREST = 0x2000;

    var cursor = curScore.newCursor();
    cursor.track = 0;
    cursor.rewind(0);
    cursor.rewind(1);
    if (!cursor.segment) return;

    var startTick, endTick, startTrack, endTrack, singleWindow;

    if (sel && sel.isRange) {
        // Range selection: use selection bounds
        startTick = sel.startSegment ? sel.startSegment.tick : cursor.tick;
        endTick = sel.endSegment ? sel.endSegment.tick : (curScore.lastSegment ? curScore.lastSegment.tick + 1 : 999999);
        startTrack = (sel.startStaff || 0) * 4;
        endTrack = (sel.endStaff || curScore.nstaves) * 4;
        if (endTrack > ntracks) endTrack = ntracks;

        // Check if selection is within one measure
        cursor.rewind(1);
        var measureEndTick = cursor.tick;
        if (cursor.nextMeasure()) measureEndTick = cursor.tick;
        singleWindow = (endTick <= measureEndTick);
        cursor.rewind(1);
    } else {
        // No selection: whole score
        startTick = cursor.tick;
        endTick = curScore.lastSegment ? curScore.lastSegment.tick + 1 : 999999;
        startTrack = 0;
        endTrack = ntracks;
        singleWindow = false;
    }

    curScore.startCmd();

    var noteInfos = [];
    var segment = cursor.segment;
    var measureStartTick = segment.tick;
    if (!singleWindow) cursor.nextMeasure();
    var measureTicks = (!singleWindow && cursor.tick > measureStartTick) ? (cursor.tick - measureStartTick) : 1920;
    var inLastMeasure = false;
    var crCount = 0;
    var totalChords = 0;
    var totalNotes = 0;

    while (segment && segment.tick < endTick) {
        if (segment.segmentType !== SEGMENT_CHORDREST) {
            segment = segment.next;
            continue;
        }
        if (segment.tick < startTick) {
            segment = segment.next;
            continue;
        }
        crCount++;

        if (singleWindow) {
            // Single window: collect all notes, process once at end
            for (var track = startTrack; track < endTrack; track++) {
                var el = segment.elementAt(track);
                if (!el || !el.notes || el.notes.length === 0) continue;
                totalChords++;
                var chord = el;
                var segTick = segment.tick;
                var durTicks = (chord.duration && chord.duration.ticks) ? chord.duration.ticks : 480;
                if (chord.graceNotes && chord.graceNotes.length > 0) {
                    for (var g = 0; g < chord.graceNotes.length; g++) {
                        var gChord = chord.graceNotes[g];
                        if (gChord.notes) {
                            for (var n = 0; n < gChord.notes.length; n++) {
                                var note = gChord.notes[n];
                                noteInfos.push({ note: note, pitch: note.pitch, tick: segTick, durationTicks: 120 });
                            }
                        }
                    }
                }
                if (chord.notes) {
                    for (var n = 0; n < chord.notes.length; n++) {
                        var note = chord.notes[n];
                        noteInfos.push({ note: note, pitch: note.pitch, tick: segTick, durationTicks: durTicks });
                        totalNotes++;
                    }
                }
            }
        } else {
            // Measure by measure
            if (!inLastMeasure && segment.tick >= cursor.tick) {
                processMeasureNotes(noteInfos, measureTicks, measureStartTick, keysig);
                noteInfos = [];
                measureStartTick = segment.tick;
                measureTicks = cursor.tick > measureStartTick ? (cursor.tick - measureStartTick) : 1920;
                if (!cursor.nextMeasure()) inLastMeasure = true;
            }

            for (var track = startTrack; track < endTrack; track++) {
                var el = segment.elementAt(track);
                if (!el || !el.notes || el.notes.length === 0) continue;
                totalChords++;
                var chord = el;
                var segTick = segment.tick;
                var durTicks = (chord.duration && chord.duration.ticks) ? chord.duration.ticks : 480;
                if (chord.graceNotes && chord.graceNotes.length > 0) {
                    for (var g = 0; g < chord.graceNotes.length; g++) {
                        var gChord = chord.graceNotes[g];
                        if (gChord.notes) {
                            for (var n = 0; n < gChord.notes.length; n++) {
                                var note = gChord.notes[n];
                                noteInfos.push({ note: note, pitch: note.pitch, tick: segTick, durationTicks: 120 });
                            }
                        }
                    }
                }
                if (chord.notes) {
                    for (var n = 0; n < chord.notes.length; n++) {
                        var note = chord.notes[n];
                        noteInfos.push({ note: note, pitch: note.pitch, tick: segTick, durationTicks: durTicks });
                        totalNotes++;
                    }
                }
            }
        }
        segment = segment.next;
    }

    if (singleWindow) {
        // Process selection as single harmonic window (measureStartTick/measureTicks = selection span)
        var windowTicks = Math.max(endTick - startTick, 480);
        processMeasureNotes(noteInfos, windowTicks, startTick, keysig);
    } else {
        processMeasureNotes(noteInfos, measureTicks, measureStartTick, keysig);
    }

    log("colordegrees: singleWindow=" + singleWindow + " crSegments=" + crCount + " chords=" + totalChords + " notes=" + totalNotes);
    curScore.endCmd();
}
