/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-CLA-applies
 *
 * MuseScore
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore BVBA and others
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

#include "utils.h"

#include <cmath>
#include <map>

#include "containers.h"

#include "chord.h"
#include "chordrest.h"
#include "clef.h"
#include "measure.h"
#include "note.h"
#include "page.h"
#include "part.h"
#include "pitchspelling.h"
#include "rest.h"
#include "score.h"
#include "segment.h"
#include "sig.h"
#include "staff.h"
#include "system.h"
#include "tuplet.h"

#include "log.h"

using namespace mu;

namespace mu::engraving {
//---------------------------------------------------------
//   handleRect
//---------------------------------------------------------

RectF handleRect(const PointF& pos)
{
    return RectF(pos.x() - 4, pos.y() - 4, 8, 8);
}

//---------------------------------------------------------
//   tick2measure
//---------------------------------------------------------

Measure* Score::tick2measure(const Fraction& tick) const
{
    if (tick == Fraction(-1, 1)) {   // special number
        return lastMeasure();
    }
    if (tick <= Fraction(0, 1)) {
        return firstMeasure();
    }

    Measure* lm = 0;
    for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
        if (tick < m->tick()) {
            assert(lm);
            return lm;
        }
        lm = m;
    }
    // check last measure
    if (lm && (tick >= lm->tick()) && (tick <= lm->endTick())) {
        return lm;
    }
    LOGD("tick2measure %d (max %d) not found", tick.ticks(), lm ? lm->tick().ticks() : -1);
    return 0;
}

//---------------------------------------------------------
//   tick2measureMM
//---------------------------------------------------------

Measure* Score::tick2measureMM(const Fraction& t) const
{
    Fraction tick(t);
    if (tick == Fraction(-1, 1)) {
        return lastMeasureMM();
    }
    if (tick < Fraction(0, 1)) {
        tick = Fraction(0, 1);
    }

    Measure* lm = 0;

    for (Measure* m = firstMeasureMM(); m; m = m->nextMeasureMM()) {
        if (tick < m->tick()) {
            assert(lm);
            return lm;
        }
        lm = m;
    }
    // check last measure
    if (lm && (tick >= lm->tick()) && (tick <= lm->endTick())) {
        return lm;
    }
    LOGD("tick2measureMM %d (max %d) not found", tick.ticks(), lm ? lm->tick().ticks() : -1);
    return 0;
}

//---------------------------------------------------------
//   tick2measureBase
//---------------------------------------------------------

MeasureBase* Score::tick2measureBase(const Fraction& tick) const
{
    for (MeasureBase* mb = first(); mb; mb = mb->next()) {
        Fraction st = mb->tick();
        Fraction l  = mb->ticks();
        if (tick >= st && tick < (st + l)) {
            return mb;
        }
    }
//      LOGD("tick2measureBase %d not found", tick);
    return 0;
}

//---------------------------------------------------------
//   tick2segment
//---------------------------------------------------------

Segment* Score::tick2segmentMM(const Fraction& tick, bool first, SegmentType st) const
{
    return tick2segment(tick, first, st, true);
}

Segment* Score::tick2segmentMM(const Fraction& tick) const
{
    return tick2segment(tick, false, SegmentType::All, true);
}

Segment* Score::tick2segmentMM(const Fraction& tick, bool first) const
{
    return tick2segment(tick, first, SegmentType::All, true);
}

Segment* Score::tick2segment(const Fraction& t, bool first, SegmentType st, bool useMMrest) const
{
    Fraction tick(t);
    Measure* m;
    if (useMMrest) {
        m = tick2measureMM(tick);
        // When mmRest force tick to the first segment of mmRest.
        if (m && m->isMMRest() && tick != m->endTick()) {
            tick = m->tick();
        }
    } else {
        m = tick2measure(tick);
    }

    if (m == 0) {
        LOGD("no measure for tick %d", tick.ticks());
        return 0;
    }
    for (Segment* segment   = m->first(st); segment;) {
        Fraction t1       = segment->tick();
        Segment* nsegment = segment->next(st);
        if (tick == t1) {
            if (first) {
                return segment;
            } else {
                if (!nsegment || tick < nsegment->tick()) {
                    return segment;
                }
            }
        }
        segment = nsegment;
    }
    LOGD("no segment for tick %d (start search at %d (measure %d))", tick.ticks(), t.ticks(), m->tick().ticks());
    return 0;
}

Segment* Score::tick2segment(const Fraction& tick) const
{
    return tick2segment(tick, false, SegmentType::All, false);
}

Segment* Score::tick2segment(const Fraction& tick, bool first) const
{
    return tick2segment(tick, first, SegmentType::All, false);
}

//---------------------------------------------------------
//   tick2leftSegment
/// return the segment at this tick position if any or
/// the first segment *before* this tick position
//---------------------------------------------------------

Segment* Score::tick2leftSegment(const Fraction& tick, bool useMMrest) const
{
    Measure* m = useMMrest ? tick2measureMM(tick) : tick2measure(tick);
    if (m == 0) {
        LOGD("tick2leftSegment(): not found tick %d", tick.ticks());
        return 0;
    }
    // loop over all segments
    Segment* ps = 0;
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        if (tick < s->tick()) {
            return ps;
        } else if (tick == s->tick()) {
            return s;
        }
        ps = s;
    }
    return ps;
}

//---------------------------------------------------------
//   tick2rightSegment
/// return the segment at this tick position if any or
/// the first segment *after* this tick position
//---------------------------------------------------------

Segment* Score::tick2rightSegment(const Fraction& tick, bool useMMrest) const
{
    Measure* m = useMMrest ? tick2measureMM(tick) : tick2measure(tick);
    if (m == 0) {
        //LOGD("tick2nearestSegment(): not found tick %d", tick.ticks());
        return 0;
    }
    // loop over all segments
    for (Segment* s = m->first(SegmentType::ChordRest); s; s = s->next1(SegmentType::ChordRest)) {
        if (tick <= s->tick()) {
            return s;
        }
    }
    return 0;
}

//---------------------------------------------------------
//   tick2beatType
//---------------------------------------------------------

BeatType Score::tick2beatType(const Fraction& tick) const
{
    Measure* m = tick2measure(tick);
    Fraction msrTick = m->tick();
    TimeSigFrac timeSig = sigmap()->timesig(msrTick).nominal();

    int rtick = (tick - msrTick).ticks();

    if (m->isAnacrusis()) { // measure is incomplete (anacrusis)
        rtick += timeSig.ticksPerMeasure() - m->ticks().ticks();
    }

    return timeSig.rtick2beatType(rtick);
}

//---------------------------------------------------------
//   getStaff
//---------------------------------------------------------

int getStaff(System* system, const PointF& p)
{
    PointF pp = p - system->page()->pos() - system->pos();
    for (size_t i = 0; i < system->page()->score()->nstaves(); ++i) {
        double sp = system->spatium();
        RectF r = system->bboxStaff(static_cast<int>(i)).adjusted(0.0, -sp, 0.0, sp);
        if (r.contains(pp)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

//---------------------------------------------------------
//   nextSeg
//---------------------------------------------------------

Fraction Score::nextSeg(const Fraction& tick, int track)
{
    Segment* seg = tick2segment(tick);
    while (seg) {
        seg = seg->next1(SegmentType::ChordRest);
        if (seg == 0) {
            break;
        }
        if (seg->element(track)) {
            break;
        }
    }
    return seg ? seg->tick() : Fraction(-1, 1);
}

//---------------------------------------------------------
//   nextSeg1
//---------------------------------------------------------

Segment* nextSeg1(Segment* seg, track_idx_t& track)
{
    staff_idx_t staffIdx   = track / VOICES;
    track_idx_t startTrack = staffIdx * VOICES;
    track_idx_t endTrack   = startTrack + VOICES;
    while ((seg = seg->next1(SegmentType::ChordRest))) {
        for (track_idx_t t = startTrack; t < endTrack; ++t) {
            if (seg->element(t)) {
                track = t;
                return seg;
            }
        }
    }
    return 0;
}

//---------------------------------------------------------
//   prevSeg1
//---------------------------------------------------------

Segment* prevSeg1(Segment* seg, track_idx_t& track)
{
    staff_idx_t staffIdx = track / VOICES;
    track_idx_t startTrack = staffIdx * VOICES;
    track_idx_t endTrack   = startTrack + VOICES;
    while ((seg = seg->prev1(SegmentType::ChordRest))) {
        for (track_idx_t t = startTrack; t < endTrack; ++t) {
            if (seg->element(t)) {
                track = t;
                return seg;
            }
        }
    }
    return 0;
}

//---------------------------------------------------------
//    next/prevChordNote
//
//    returns the top note of the next/previous chord. If a
//    chord exists in the same track as note,
//    it is used. If not, the topmost existing chord is used.
//    May return nullptr if there is no next/prev note
//---------------------------------------------------------

Note* nextChordNote(Note* note)
{
    track_idx_t track       = note->track();
    track_idx_t fromTrack   = (track / VOICES) * VOICES;
    track_idx_t toTrack     = fromTrack + VOICES;
    // TODO : limit to same instrument, not simply to same staff!
    Segment* seg   = note->chord()->segment()->nextCR(track, true);
    while (seg) {
        EngravingItem* targetElement = seg->elementAt(track);
        // if a chord exists in the same track, return its top note
        if (targetElement && targetElement->isChord()) {
            return toChord(targetElement)->upNote();
        }
        // if not, return topmost chord in track range
        for (track_idx_t i = fromTrack; i < toTrack; i++) {
            targetElement = seg->elementAt(i);
            if (targetElement && targetElement->isChord()) {
                return toChord(targetElement)->upNote();
            }
        }
        seg = seg->nextCR(track, true);
    }
    return nullptr;
}

Note* prevChordNote(Note* note)
{
    track_idx_t track       = note->track();
    track_idx_t fromTrack   = (track / VOICES) * VOICES;
    track_idx_t toTrack     = fromTrack + VOICES;
    // TODO : limit to same instrument, not simply to same staff!
    Segment* seg   = note->chord()->segment()->prev1();
    while (seg) {
        if (seg->segmentType() == SegmentType::ChordRest) {
            EngravingItem* targetElement = seg->elementAt(track);
            // if a chord exists in the same track, return its top note
            if (targetElement && targetElement->isChord()) {
                return toChord(targetElement)->upNote();
            }
            // if not, return topmost chord in track range
            for (track_idx_t i = fromTrack; i < toTrack; i++) {
                targetElement = seg->elementAt(i);
                if (targetElement && targetElement->isChord()) {
                    return toChord(targetElement)->upNote();
                }
            }
        }
        seg = seg->prev1();
    }
    return nullptr;
}

//---------------------------------------------------------
//   pitchKeyAdjust
//    change entered note to sounding pitch dependent
//    on key.
//    Example: if F is entered in G-major, a Fis is played
//    key -7 ... +7
//---------------------------------------------------------

int pitchKeyAdjust(int step, Key key)
{
    static int ptab[15][7] = {
//             c  d  e  f  g   a  b
        { -1, 1, 3, 4, 6,  8, 10 },         // Bes
        { -1, 1, 3, 5, 6,  8, 10 },         // Ges
        { 0, 1, 3, 5, 6,  8, 10 },          // Des
        { 0, 1, 3, 5, 7,  8, 10 },          // As
        { 0, 2, 3, 5, 7,  8, 10 },          // Es
        { 0, 2, 3, 5, 7,  9, 10 },          // B
        { 0, 2, 4, 5, 7,  9, 10 },          // F
        { 0, 2, 4, 5, 7,  9, 11 },          // C
        { 0, 2, 4, 6, 7,  9, 11 },          // G
        { 1, 2, 4, 6, 7,  9, 11 },          // D
        { 1, 2, 4, 6, 8,  9, 11 },          // A
        { 1, 3, 4, 6, 8,  9, 11 },          // E
        { 1, 3, 4, 6, 8, 10, 11 },          // H
        { 1, 3, 5, 6, 8, 10, 11 },          // Fis
        { 1, 3, 5, 6, 8, 10, 12 },          // Cis
    };
    return ptab[int(key) + 7][step];
}

//---------------------------------------------------------
//   y2pitch
//---------------------------------------------------------

int y2pitch(double y, ClefType clef, double _spatium)
{
    int l = lrint(y / _spatium * 2.0);
    return line2pitch(l, clef, Key::C);
}

//---------------------------------------------------------
//   line2pitch
//    key  -7 ... +7
//---------------------------------------------------------

int line2pitch(int line, ClefType clef, Key key)
{
    int l      = ClefInfo::pitchOffset(clef) - line;
    int octave = 0;
    if (l < 0) {
        l = 0;
    }

    octave += l / 7;
    l       = l % 7;

    int pitch = pitchKeyAdjust(l, key) + octave * 12;

    if (pitch > 127) {
        pitch = 127;
    } else if (pitch < 0) {
        pitch = 0;
    }
    return pitch;
}

//---------------------------------------------------------
//   quantizeLen
//---------------------------------------------------------

int quantizeLen(int len, int raster)
{
    if (raster == 0) {
        return len;
    }
    return int(((float)len / raster) + 0.5) * raster;   //round to the closest multiple of raster
}

static const char16_t* vall[] = {
    u"c",
    u"c♯",
    u"d",
    u"d♯",
    u"e",
    u"f",
    u"f♯",
    u"g",
    u"g♯",
    u"a",
    u"a♯",
    u"b"
};
static const char16_t* valu[] = {
    u"C",
    u"C♯",
    u"D",
    u"D♯",
    u"E",
    u"F",
    u"F♯",
    u"G",
    u"G♯",
    u"A",
    u"A♯",
    u"B"
};

/*!
 * Returns the string representation of the given pitch.
 *
 * Returns the latin letter name, accidental, and octave numeral.
 * Uses upper case only for pitches 0-24.
 *
 * @param v
 *  The pitch number of the note.
 *
 * @return
 *  The string representation of the note.
 */
String pitch2string(int v)
{
    if (v < 0 || v > 127) {
        return String(u"----");
    }
    int octave = (v / 12) - 1;
    String o;
    o = String::number(octave);
    int i = v % 12;
    return (octave < 0 ? valu[i] : vall[i]) + o;
}

/*!
 * An array of all supported interval sorted by size.
 *
 * Because intervals can be spelled differently, this array
 * tracks all the different valid intervals. They are arranged
 * in diatonic then chromatic order.
 */
Interval intervalList[intervalListSize] = {
    // diatonic - chromatic
    Interval(0, 0),           //  0 Perfect Unison
    Interval(0, 1),           //  1 Augmented Unison

    Interval(1, 0),           //  2 Diminished Second
    Interval(1, 1),           //  3 Minor Second
    Interval(1, 2),           //  4 Major Second
    Interval(1, 3),           //  5 Augmented Second

    Interval(2, 2),           //  6 Diminished Third
    Interval(2, 3),           //  7 Minor Third
    Interval(2, 4),           //  8 Major Third
    Interval(2, 5),           //  9 Augmented Third

    Interval(3, 4),           // 10 Diminished Fourth
    Interval(3, 5),           // 11 Perfect Fourth
    Interval(3, 6),           // 12 Augmented Fourth

    Interval(4, 6),           // 13 Diminished Fifth
    Interval(4, 7),           // 14 Perfect Fifth
    Interval(4, 8),           // 15 Augmented Fifth

    Interval(5, 7),           // 16 Diminished Sixth
    Interval(5, 8),           // 17 Minor Sixth
    Interval(5, 9),           // 18 Major Sixth
    Interval(5, 10),          // 19 Augmented Sixth

    Interval(6, 9),           // 20 Diminished Seventh
    Interval(6, 10),          // 21 Minor Seventh
    Interval(6, 11),          // 22 Major Seventh
    Interval(6, 12),          // 23 Augmented Seventh

    Interval(7, 11),          // 24 Diminished Octave
    Interval(7, 12)           // 25 Perfect Octave
};

/*!
 * Finds the most likely diatonic interval for a semitone distance.
 *
 * Uses the most common diatonic intervals.
 *
 * @param
 *  The number of semitones in the chromatic interval.
 *  Negative semitones will simply be made positive.
 *
 * @return
 *  The number of diatonic steps in the interval.
 */

int chromatic2diatonic(int semitones)
{
    static int il[12] = {
        0,        // Perfect Unison
        3,        // Minor Second
        4,        // Major Second
        7,        // Minor Third
        8,        // Major Third
        11,       // Perfect Fourth
        12,       // Augmented Fourth
        14,       // Perfect Fifth
        17,       // Minor Sixth
        18,       // Major Sixth
        21,       // Minor Seventh
        22,       // Major Seventh
        // 25    Perfect Octave
    };
    bool down = semitones < 0;
    if (down) {
        semitones = -semitones;
    }
    int val = semitones % 12;
    int octave = semitones / 12;
    int intervalIndex = il[val];
    int steps = intervalList[intervalIndex].diatonic;
    steps = steps + octave * 7;
    return down ? -steps : steps;
}

//---------------------------------------------------------
//   searchInterval
//---------------------------------------------------------

int searchInterval(int steps, int semitones)
{
    unsigned n = sizeof(intervalList) / sizeof(*intervalList);
    for (unsigned i = 0; i < n; ++i) {
        if ((intervalList[i].diatonic == steps) && (intervalList[i].chromatic == semitones)) {
            return i;
        }
    }
    return -1;
}

//---------------------------------------------------------
//   diatonicUpDown
//    used to find the second note of a trill, mordent etc.
//    key  -7 ... +7
//---------------------------------------------------------

int diatonicUpDown(Key k, int pitch, int steps)
{
    static int ptab[15][7] = {
//             c  c#   d  d#    e   f  f#   g  g#  a  a#   b
        { -1,      1,       3,  4,      6,     8,      10 },         // Cb Ces
        { -1,      1,       3,  5,      6,     8,      10 },         // Gb Ges
        { 0,      1,       3,  5,      6,     8,      10 },          // Db Des
        { 0,      1,       3,  5,      7,     8,      10 },          // Ab As
        { 0,      2,       3,  5,      7,     8,      10 },          // Eb Es
        { 0,      2,       3,  5,      7,     9,      10 },          // Bb B
        { 0,      2,       4,  5,      7,     9,      10 },          // F  F

        { 0,      2,       4,  5,      7,     9,      11 },          // C  C

        { 0,      2,       4,  6,      7,     9,      11 },          // G  G
        { 1,      2,       4,  6,      7,     9,      11 },          // D  D
        { 1,      2,       4,  6,      8,     9,      11 },          // A  A
        { 1,      3,       4,  6,      8,     9,      11 },          // E  E
        { 1,      3,       4,  6,      8,    10,      11 },          // B  H
        { 1,      3,       5,  6,      8,    10,      11 },          // F# Fis
        { 1,      3,       5,  6,      8,    10,      12 },          // C# Cis
    };

    int key    = int(k) + 7;
    int step   = pitch % 12;
    int octave = pitch / 12;

    // loop through the diatonic steps of the key looking for the given note
    // or the gap where it would fit
    int i = 0;
    while (i < 7) {
        if (ptab[key][i] >= step) {
            break;
        }
        ++i;
    }

    // neither step nor gap found
    // reset to beginning
    if (i == 7) {
        ++octave;
        i = 0;
    }
    // if given step not found (gap found instead), and we are stepping up
    // then we've already accounted for one step
    if (ptab[key][i] > step && steps > 0) {
        --steps;
    }

    // now start counting diatonic steps up or down
    if (steps > 0) {
        // count up
        while (steps--) {
            ++i;
            if (i == 7) {
                // hit last step; reset to beginning
                ++octave;
                i = 0;
            }
        }
    } else if (steps < 0) {
        // count down
        while (steps++) {
            --i;
            if (i < 0) {
                // hit first step; reset to end
                --octave;
                i = 6;
            }
        }
    }

    // convert step to pitch
    step = ptab[key][i];
    pitch = octave * 12 + step;
    if (pitch < 0) {
        pitch = 0;
    }
    if (pitch > 127) {
        pitch = 128;
    }
    return pitch;
}

//---------------------------------------------------------
//   searchTieNote
//    search Note to tie to "note"
//---------------------------------------------------------

Note* searchTieNote(Note* note)
{
    if (!note) {
        return nullptr;
    }

    Note* note2  = 0;
    Chord* chord = note->chord();
    Segment* seg = chord->segment();
    Part* part   = chord->part();
    track_idx_t strack = part->staves().front()->idx() * VOICES;
    track_idx_t etrack = strack + part->staves().size() * VOICES;

    if (chord->isGraceBefore()) {
        chord = toChord(chord->explicitParent());

        // try to tie to next grace note

        size_t index = note->chord()->graceIndex();
        for (Chord* c : chord->graceNotes()) {
            if (c->graceIndex() == index + 1) {
                note2 = c->findNote(note->pitch());
                if (note2) {
//printf("found grace-grace tie\n");
                    return note2;
                }
            }
        }

        // try to tie to note in parent chord
        note2 = chord->findNote(note->pitch());
        if (note2) {
            return note2;
        }
    } else if (chord->isGraceAfter()) {
        // grace after
        // we will try to tie to note in next normal chord, below
        // meanwhile, set chord to parent chord so the endTick calculation will make sense
        chord = toChord(chord->explicitParent());
    } else {
        // normal chord
        // try to tie to grace note after if present
        std::vector<Chord*> gna = chord->graceNotesAfter();
        if (!gna.empty()) {
            Chord* gc = gna[0];
            note2 = gc->findNote(note->pitch());
            if (note2) {
                return note2;
            }
        }
    }
    // at this point, chord is a regular chord, not a grace chord
    // and we are looking for a note in the *next* chord (grace or regular)

    // calculate end of current note duration
    // but err on the safe side in case there is roundoff in tick count
    Fraction endTick = chord->tick() + chord->actualTicks() - Fraction(1, 4 * 480);

    int idx1 = note->unisonIndex();
    while ((seg = seg->next1(SegmentType::ChordRest))) {
        // skip ahead to end of current note duration as calculated above
        // but just in case, stop if we find element in current track
        if (seg->tick() < endTick && !seg->element(chord->track())) {
            continue;
        }
        for (track_idx_t track = strack; track < etrack; ++track) {
            EngravingItem* e = seg->element(track);
            if (e == 0 || !e->isChord()) {
                continue;
            }
            Chord* c = toChord(e);
            const staff_idx_t staffIdx = c->staffIdx() + c->staffMove();
            if (staffIdx != chord->staffIdx() + chord->staffMove()) {
                // this check is needed as we are iterating over all staves to capture cross-staff chords
                continue;
            }
            // if there are grace notes before, try to tie to first one
            std::vector<Chord*> gnb = c->graceNotesBefore();
            if (!gnb.empty()) {
                Chord* gc = gnb[0];
                Note* gn2 = gc->findNote(note->pitch());
                if (gn2) {
                    return gn2;
                }
            }
            int idx2 = 0;
            for (Note* n : c->notes()) {
                if (n->pitch() == note->pitch()) {
                    if (idx1 == idx2) {
                        if (note2 == 0 || c->track() == chord->track()) {
                            note2 = n;
                            break;
                        }
                    } else {
                        ++idx2;
                    }
                }
            }
        }
        if (note2) {
            break;
        }
    }
    return note2;
}

//---------------------------------------------------------
//   searchTieNote114
//    search Note to tie to "note", tie to next note in
//    same voice
//---------------------------------------------------------

Note* searchTieNote114(Note* note)
{
    Note* note2  = 0;
    Chord* chord = note->chord();
    Segment* seg = chord->segment();
    Part* part   = chord->part();
    track_idx_t strack = part->staves().front()->idx() * VOICES;
    track_idx_t etrack = strack + part->staves().size() * VOICES;

    while ((seg = seg->next1(SegmentType::ChordRest))) {
        for (track_idx_t track = strack; track < etrack; ++track) {
            EngravingItem* e = seg->element(track);
            if (e == 0 || (!e->isChord()) || (e->track() != chord->track())) {
                continue;
            }
            Chord* c = toChord(e);
            staff_idx_t staffIdx = c->staffIdx() + c->staffMove();
            if (staffIdx != chord->staffIdx() + chord->staffMove()) {      // cannot happen?
                continue;
            }
            for (Note* n : c->notes()) {
                if (n->pitch() == note->pitch()) {
                    if (note2 == 0 || c->track() == chord->track()) {
                        note2 = n;
                    }
                }
            }
        }
        if (note2) {
            break;
        }
    }
    return note2;
}

//---------------------------------------------------------
//   absStep
///   Compute absolute step.
///   C D E F G A B ....
//---------------------------------------------------------

int absStep(int tpc, int pitch)
{
    int line     = tpc2step(tpc) + (pitch / 12) * 7;
    int tpcPitch = tpc2pitch(tpc);

    if (tpcPitch < 0) {
        line += 7;
    } else {
        line -= (tpcPitch / 12) * 7;
    }
    return line;
}

int absStep(int pitch)
{
    // TODO - does this need to be key-aware?
    int tpc = pitch2tpc(pitch, Key::C, Prefer::NEAREST);
    return absStep(tpc, pitch);
}

int absStep(int line, ClefType clef)
{
    return ClefInfo::pitchOffset(clef) - line;
}

//---------------------------------------------------------
//   relStep
///   Compute relative step from absolute step
///   which depends on actual clef. Step 0 starts on the
///   first (top) staff line.
//---------------------------------------------------------

int relStep(int line, ClefType clef)
{
    return ClefInfo::pitchOffset(clef) - line;
}

int relStep(int pitch, int tpc, ClefType clef)
{
    return relStep(absStep(tpc, pitch), clef);
}

//---------------------------------------------------------
//   pitch2step
//   returns one of { 0, 1, 2, 3, 4, 5, 6 }
//---------------------------------------------------------

int pitch2step(int pitch)
{
    //                            C  C# D  D# E  F  F# G  G# A  A# B
    static const char tab[12] = { 0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6 };
    return tab[pitch % 12];
}

//---------------------------------------------------------
//   step2pitch
//   returns one of { 0, 2, 4, 5, 7, 9, 11 }
//---------------------------------------------------------

int step2pitch(int step)
{
    static const char tab[7] = { 0, 2, 4, 5, 7, 9, 11 };
    return tab[step % 7];
}

//---------------------------------------------------------
//   convertLine
// find the line in clefF corresponding to lineL2 in clefR
//---------------------------------------------------------

int convertLine(int lineL2, ClefType clefL, ClefType clefR)
{
    int lineR2 = lineL2;
    int goalpitch = line2pitch(lineL2, clefL, Key::C);
    int p;
    while ((p = line2pitch(lineR2, clefR, Key::C)) > goalpitch && p < 127) {
        lineR2++;
    }
    while ((p = line2pitch(lineR2, clefR, Key::C)) < goalpitch && p > 0) {
        lineR2--;
    }
    return lineR2;
}

//---------------------------------------------------------
//   convertLine
// find the line in clef for NoteL corresponding to lineL2 in clef for noteR
// for example middle C is line 10 in Treble clef, but is line -2 in Bass clef.
//---------------------------------------------------------

int convertLine(int lineL2, const Note* noteL, const Note* noteR)
{
    return convertLine(lineL2,
                       noteL->chord()->staff()->clef(noteL->chord()->tick()),
                       noteR->chord()->staff()->clef(noteR->chord()->tick()));
}

//---------------------------------------------------------
//   chromaticPitchSteps -- an articulation such as a trill, or mordant consists of several notes
// played in succession.  The pitch offsets of each such note in the sequence can be represented either
// as a number of steps in the diatonic scale, or in half steps as on a piano keyboard.
// this function, articulationExcursion, takes deltastep indicating the number of steps in the
// diatonic scale, and calculates (and returns) the number of half steps, taking several things into account.
// E.g., the key signature, a trill from e to f, is to be understood as a trill between E and F# if we are
// in the key of G.
// E.g., if previously (looking backward in time) in the same measure there is another note on the same
// staff line/space, and that note has an accidental (sharp,flat,natural,etc), then we want to match that
// tone exactly.
// E.g., If there are multiple notes on the same line/space, then we only consider the most
// recent one, but avoid looking forward in time after the current note.
// E.g., Also if there is an accidental     // on a note one (or more) octaves above or below we
// observe its accidental as well.
// E.g., Still another case is that if two staves are involved (such as a glissando between two
// notes on different staves) then we have to search both staves for the most recent accidental.
//
// noteL is the note to measure the deltastep from, i.e., ornaments are w.r.t. this note
// noteR is the note to search backward from to find accidentals.
//    for ornament calculation noteL and noteR are the same, but for glissando they are
//     the start and end note of glissando.
// nominalDiatonicSteps is the desired number of diatonic steps between the base note and this articulation step.
//---------------------------------------------------------

int chromaticPitchSteps(const Note* noteL, const Note* noteR, const int nominalDiatonicSteps)
{
    if (0 == nominalDiatonicSteps) {
        return 0;
    }
    Chord* chordL = noteL->chord();
    Chord* chordR = noteR->chord();
    int epitchL = noteL->epitch();
    Fraction tickL = chordL->tick();
    // we cannot use staffL = chord->staff() because that won't correspond to the noteL->line()
    //   in the case the user has pressed Shift-Cmd->Up or Shift-Cmd-Down.
    //   Therefore we have to take staffMove() into account using vStaffIdx().
    Staff* staffL = noteL->score()->staff(chordL->vStaffIdx());
    ClefType clefL = staffL->clef(tickL);
    // line represents the ledger line of the staff.  0 is the top line, 1, is the space between the top 2 lines,
    //  ... 8 is the bottom line.
    int lineL = noteL->line();
    if (lineL == INVALID_LINE) {
        int relLine = absStep(noteL->tpc(), noteL->epitch());
        ClefType clef = noteL->staff()->clef(noteL->tick());
        lineL = relStep(relLine, clef);
    }

    // we use line - deltastep, because lines are oriented from top to bottom, while step is oriented from bottom to top.
    int lineL2    = lineL - nominalDiatonicSteps;
    Measure* measureR = chordR->segment()->measure();

    Segment* segment = noteL->chord()->segment();
    int lineR2 = convertLine(lineL2, noteL, noteR);
    // is there another note in this segment on the same line?
    // if so, use its pitch exactly.
    int halfsteps = 0;
    staff_idx_t staffIdx = noteL->chord()->staff()->idx();   // cannot use staffL->idx() because of staffMove()
    track_idx_t startTrack = staffIdx * VOICES;
    track_idx_t endTrack   = startTrack + VOICES;
    bool done = false;
    for (track_idx_t track = startTrack; track < endTrack; ++track) {
        EngravingItem* e = segment->element(track);
        if (!e || e->type() != ElementType::CHORD) {
            continue;
        }
        Chord* chord = toChord(e);
        if (chord->vStaffIdx() != chordL->vStaffIdx()) {
            continue;
        }
        if (!done) {
            if (staffL->isPitchedStaff(segment->tick())) {
                bool error = false;
                AccidentalVal acciv2 = measureR->findAccidental(chordR->segment(), chordR->vStaffIdx(), lineR2, error);
                int acci2 = int(acciv2);
                // epitch (effective pitch) is a visible pitch so line2pitch returns exactly that.
                halfsteps = line2pitch(lineL - nominalDiatonicSteps, clefL, Key::C) + acci2 - epitchL;
            } else {
                // cannot rely on accidentals or key signatures
                halfsteps = nominalDiatonicSteps;
            }
        }
    }
    return halfsteps;
}

//---------------------------------------------------------
//   skipTuplet
//    return segment of rightmost chord/rest in a
//    (possible nested) tuplet
//---------------------------------------------------------

Segment* skipTuplet(Tuplet* tuplet)
{
    DurationElement* nde = tuplet->elements().back();
    while (nde->isTuplet()) {
        tuplet = toTuplet(nde);
        nde = tuplet->elements().back();
    }
    return toChordRest(nde)->segment();
}

//---------------------------------------------------------
//   timeSigSymIdsFromString
//    replace ascii with bravura symbols
//---------------------------------------------------------

SymIdList timeSigSymIdsFromString(const String& string)
{
    static const std::map<Char, SymId> dict = {
        { 43,    SymId::timeSigPlusSmall },             // '+'
        { 48,    SymId::timeSig0 },                     // '0'
        { 49,    SymId::timeSig1 },                     // '1'
        { 50,    SymId::timeSig2 },                     // '2'
        { 51,    SymId::timeSig3 },                     // '3'
        { 52,    SymId::timeSig4 },                     // '4'
        { 53,    SymId::timeSig5 },                     // '5'
        { 54,    SymId::timeSig6 },                     // '6'
        { 55,    SymId::timeSig7 },                     // '7'
        { 56,    SymId::timeSig8 },                     // '8'
        { 57,    SymId::timeSig9 },                     // '9'
        { 67,    SymId::timeSigCommon },                // 'C'
        { 40,    SymId::timeSigParensLeftSmall },       // '('
        { 41,    SymId::timeSigParensRightSmall },      // ')'
        { 162,   SymId::timeSigCutCommon },             // '¢'
        { 189,   SymId::timeSigFractionHalf },
        { 188,   SymId::timeSigFractionQuarter },
        { 59664, SymId::mensuralProlation1 },
        { 79,    SymId::mensuralProlation2 },           // 'O'
        { 59665, SymId::mensuralProlation2 },
        { 216,   SymId::mensuralProlation3 },           // 'Ø'
        { 59666, SymId::mensuralProlation3 },
        { 59667, SymId::mensuralProlation4 },
        { 59668, SymId::mensuralProlation5 },
        { 59670, SymId::mensuralProlation7 },
        { 59671, SymId::mensuralProlation8 },
        { 59673, SymId::mensuralProlation10 },
        { 59674, SymId::mensuralProlation11 },
    };

    SymIdList list;
    for (size_t i = 0; i < string.size(); ++i) {
        SymId sym = mu::value(dict, string.at(i), SymId::noSym);
        if (sym != SymId::noSym) {
            list.push_back(sym);
        }
    }
    return list;
}

//---------------------------------------------------------
//   actualTicks
//---------------------------------------------------------

Fraction actualTicks(Fraction duration, Tuplet* tuplet, Fraction timeStretch)
{
    Fraction f = duration / timeStretch;
    for (Tuplet* t = tuplet; t; t = t->tuplet()) {
        f /= t->ratio();
    }
    return f;
}

double yStaffDifference(const System* system1, staff_idx_t staffIdx1, const System* system2, staff_idx_t staffIdx2)
{
    if (!system1 || !system2) {
        return 0.0;
    }
    const SysStaff* staff1 = system1->staff(staffIdx1);
    const SysStaff* staff2 = system2->staff(staffIdx2);
    if (!staff1 || !staff2) {
        return 0.0;
    }
    return staff1->y() - staff2->y();
}

bool allowRemoveWhenRemovingStaves(EngravingItem* item, staff_idx_t startStaff, staff_idx_t endStaff)
{
    // Sanity checks
    if (!item || item->staffIdx() == mu::nidx || startStaff == mu::nidx || endStaff == mu::nidx) {
        return false;
    }

    Score* score = item->score();
    if (score->nstaves() == 1) {
        return true;
    }

    if (endStaff == 0) { // Default initialized
        endStaff = startStaff + 1;
    }

    staff_idx_t staffIdx = item->staffIdx();
    if (staffIdx < startStaff || staffIdx >= endStaff) {
        return false;
    }

    Staff* nextRemaining = score->staff(endStaff);
    bool nextRemainingIsSystemObjectStaff = nextRemaining && score->isSystemObjectStaff(nextRemaining);
    if (item->isTopSystemObject() && !nextRemainingIsSystemObjectStaff) {
        return false;
    }

    return true;
}

bool moveDownWhenAddingStaves(EngravingItem* item, staff_idx_t startStaff, staff_idx_t endStaff)
{
    // Sanity checks
    if (!item || item->staffIdx() == mu::nidx || startStaff == mu::nidx || endStaff == mu::nidx) {
        return false;
    }

    if (item->staffIdx() < startStaff) {
        return false;
    }

    if (endStaff == 0) { // Default initialized
        endStaff = startStaff + 1;
    }

    Score* score = item->score();
    Staff* nextAfterInserted = score->staff(endStaff);
    bool nextAfterInsertedIsSystemObjectStaff = nextAfterInserted && score->isSystemObjectStaff(nextAfterInserted);
    if (item->isTopSystemObject() && !nextAfterInsertedIsSystemObjectStaff) {
        return false;
    }

    return true;
}

void collectChordsAndRest(Segment* segment, staff_idx_t staffIdx, std::vector<Chord*>& chords, std::vector<Rest*>& rests)
{
    if (!segment) {
        return;
    }

    track_idx_t startTrack = staffIdx * VOICES;
    track_idx_t endTrack = startTrack + VOICES;

    for (track_idx_t track = startTrack; track < endTrack; ++track) {
        EngravingItem* e = segment->elementAt(track);
        if (!e) {
            continue;
        }
        if (e->isChord() && !toChordRest(e)->staffMove()) {
            chords.push_back(toChord(e));
        } else if (e->isRest() && !toChordRest(e)->staffMove()) {
            rests.push_back(toRest(e));
        }
    }
}

void collectChordsOverlappingRests(Segment* segment, staff_idx_t staffIdx, std::vector<Chord*>& chords)
{
    // Check if previous segments contain chords in other voices
    // whose duration overlaps with rests on this segment

    track_idx_t startTrack = staffIdx * VOICES;
    track_idx_t endTrack = startTrack + VOICES;

    std::set<track_idx_t> tracksToCheck;
    for (track_idx_t track = startTrack; track < endTrack; ++track) {
        EngravingItem* item = segment->elementAt(track);
        if (!item || !item->isRest()) {
            tracksToCheck.insert(track);
        }
    }

    Fraction curTick = segment->rtick();
    for (Segment* prevSeg = segment->prev(); prevSeg; prevSeg = prevSeg->prev()) {
        if (!prevSeg->isChordRestType()) {
            continue;
        }
        Fraction prevSegTick = prevSeg->rtick();
        for (track_idx_t track : tracksToCheck) {
            EngravingItem* e = prevSeg->elementAt(track);
            if (!e || !e->isChord()) {
                continue;
            }
            Chord* chord = toChord(e);
            Fraction chordEndTick = prevSegTick + chord->actualTicks();
            if (chordEndTick <= curTick) {
                continue;
            }
            Measure* measure = segment->measure();
            Segment* endSegment = measure->findSegmentR(SegmentType::ChordRest, chordEndTick);
            if (!endSegment) {
                continue;
            }
            EngravingItem* endItem = endSegment->elementAt(track);
            if (!endItem || !endItem->isChord()) {
                continue;
            }

            chords.push_back(chord);
        }
    }
}
}
