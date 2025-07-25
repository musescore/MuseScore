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

#include "utils.h"

#include <cmath>
#include <map>

#include "containers.h"

#include "accidental.h"
#include "arpeggio.h"
#include "chord.h"
#include "chordrest.h"
#include "clef.h"
#include "laissezvib.h"
#include "lyrics.h"
#include "marker.h"
#include "masterscore.h"
#include "repeatlist.h"
#include "keysig.h"
#include "measure.h"
#include "measurenumber.h"
#include "note.h"
#include "page.h"
#include "part.h"
#include "partialtie.h"
#include "pitchspelling.h"
#include "rest.h"
#include "score.h"
#include "segment.h"
#include "sig.h"
#include "staff.h"
#include "system.h"
#include "spanner.h"
#include "tremolosinglechord.h"
#include "tremolotwochord.h"
#include "tuplet.h"
#include "drumset.h"
#include "barline.h"

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

    return m_measures.measureByTick(tick.ticks());
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

    Measure* measure = m_measures.measureByTick(t.ticks());
    if (!measure) {
        LOGD("tick2measureMM %d not found", tick.ticks());
        return nullptr;
    }

    return measure->coveringMMRestOrThis();
}

//---------------------------------------------------------
//   tick2measureBase
//---------------------------------------------------------

MeasureBase* Score::tick2measureBase(const Fraction& tick) const
{
    std::vector<MeasureBase*> mbList = m_measures.measureBasesAtTick(tick.ticks());
    for (MeasureBase* mb : mbList) {
        Fraction st = mb->tick();
        Fraction l  = mb->ticks();
        if (tick >= st && tick < (st + l)) {
            return mb;
        }
    }

    LOGD("tick2measureBase %d not found", tick.ticks());
    return nullptr;
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
        LOGD() << "no measure for tick " << tick.ticks();
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

Segment* Score::tick2leftSegment(const Fraction& tick, bool useMMrest, SegmentType segType) const
{
    Measure* m = useMMrest ? tick2measureMM(tick) : tick2measure(tick);
    if (m == 0) {
        LOGD("tick2leftSegment(): not found tick %d", tick.ticks());
        return 0;
    }

    // loop over all segments
    Segment* ps = 0;
    for (Segment* s = m->first(segType); s; s = s->next(segType)) {
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

Segment* Score::tick2rightSegment(const Fraction& tick, bool useMMrest, SegmentType segType) const
{
    Measure* m = useMMrest ? tick2measureMM(tick) : tick2measure(tick);
    if (m == 0) {
        //LOGD("tick2nearestSegment(): not found tick %d", tick.ticks());
        return 0;
    }
    // loop over all segments
    for (Segment* s = m->first(segType); s; s = s->next1(segType)) {
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

void Score::checkChordList()
{
    m_chordList.checkChordList(style());
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

Segment* nextSeg1(Segment* seg)
{
    Segment* nextSeg = seg;
    while (nextSeg && nextSeg->rtick() == seg->rtick()) {
        nextSeg = nextSeg->next1(Segment::CHORD_REST_OR_TIME_TICK_TYPE);
    }
    return nextSeg;
}

//---------------------------------------------------------
//   prevSeg1
//---------------------------------------------------------

Segment* prevSeg1(Segment* seg)
{
    Segment* prevSeg = seg;
    while (prevSeg && prevSeg->rtick() == seg->rtick()) {
        prevSeg = prevSeg->prev1(Segment::CHORD_REST_OR_TIME_TICK_TYPE);
    }
    return prevSeg;
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

static const char16_t* valSharp[] = {
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
static const char16_t* valFlat[] = {
    u"c",
    u"d♭",
    u"d",
    u"e♭",
    u"e",
    u"f",
    u"g♭",
    u"g",
    u"a♭",
    u"a",
    u"b♭",
    u"b"
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
String pitch2string(int v, bool useFlats)
{
    if (v < 0 || v > 127) {
        return String(u"----");
    }
    int octave = (v / PITCH_DELTA_OCTAVE) - 1;
    String o;
    o = String::number(octave);
    int i = v % PITCH_DELTA_OCTAVE;

    String pitchStr = useFlats ? valFlat[i] : valSharp[i];
    if (octave < 0) {
        pitchStr = pitchStr.toUpper();
    }

    return pitchStr + o;
}

/*!
 * Returns the pitch number of the given string.
 *
 * @param s
 *  The string representation of the note.
 *
 * @return
 *  The pitch number of the note.
 */
int string2pitch(const String& s)
{
    if (s == String(u"----")) {
        return -1;
    }

    String value = s;

    bool negative = s.contains(u'-');
    int octave = String(s[s.size() - 1]).toInt() * (negative ? -1 : 1);
    if (octave < -1 || octave > 9) {
        return -1;
    }

    value = value.mid(0, value.size() - (negative ? 2 : 1));
    value = value.toLower();

    int pitchIndex = -1;
    for (int i = 0; i < PITCH_DELTA_OCTAVE; ++i) {
        if (value == valFlat[i] || value == valSharp[i]) {
            pitchIndex = i;
            break;
        }
    }

    if (pitchIndex == -1) {
        return -1;
    }

    return (octave + 1) * PITCH_DELTA_OCTAVE + pitchIndex;
}

String convertPitchStringFlatsAndSharpsToUnicode(const String& str)
{
    if (str.isEmpty()) {
        return String();
    }

    String value = String(str[0]);
    for (size_t i = 1; i < str.size(); ++i) {
        Char symbol = str.at(i).toLower();
        if (symbol == u'b') {
            value.append(u'♭');
        } else if (symbol == u'#') {
            value.append(u'♯');
        } else {
            value.append(symbol);
        }
    }

    return value;
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

Note* searchTieNote(const Note* note, const Segment* nextSegment, const bool disableOverRepeats)
{
    if (!note) {
        return nullptr;
    }

    Note* note2  = nullptr;
    Chord* chord = note->chord();
    Segment* seg = chord->segment();
    Part* part   = chord->part();
    track_idx_t strack = part->staves().front()->idx() * VOICES;
    track_idx_t etrack = strack + part->staves().size() * VOICES;

    if (!nextSegment) {
        const Fraction nextTick = chord->tick() + chord->actualTicks();
        nextSegment = seg->next1(SegmentType::ChordRest);
        while (nextSegment && nextSegment->tick() < nextTick) {
            nextSegment = nextSegment->next1(SegmentType::ChordRest);
        }
    }

    if (!nextSegment) {
        return nullptr;
    }

    if (disableOverRepeats && !segmentsAreAdjacentInRepeatStructure(seg, nextSegment)) {
        return nullptr;
    }

    if (chord->isGraceBefore()) {
        chord = toChord(chord->explicitParent());

        // try to tie to next grace note

        size_t index = note->chord()->graceIndex();
        for (Chord* c : chord->graceNotes()) {
            if (c->graceIndex() == index + 1) {
                note2 = c->findNote(note->pitch());
                if (note2) {
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

    int idx1 = note->unisonIndex();
    for (track_idx_t track = strack; track < etrack; ++track) {
        EngravingItem* e = nextSegment->element(track);
        if (!e || !e->isChord()) {
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
                    if (!note2 || c->track() == chord->track()) {
                        note2 = n;
                        break;
                    }
                } else {
                    ++idx2;
                }
            }
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
        int absLine = absStep(noteL->tpc(), noteL->epitch());
        ClefType clef = noteL->staff()->clef(noteL->tick());
        lineL = relStep(absLine, clef);
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

static void noteValToEffectivePitchAndTpc(const NoteVal& nval, const Staff* staff, const Fraction& tick, int& epitch, int& tpc)
{
    const bool concertPitch = staff->concertPitch();

    if (concertPitch) {
        epitch = nval.pitch;
    } else {
        const int pitchOffset = staff->part()->instrument(tick)->transpose().chromatic;
        epitch = nval.pitch - pitchOffset;
    }

    tpc = nval.tpc(concertPitch);
    if (tpc == static_cast<int>(mu::engraving::Tpc::TPC_INVALID)) {
        tpc = pitch2tpc(epitch, staff->key(tick), mu::engraving::Prefer::NEAREST);
    }
}

int noteValToLine(const NoteVal& nval, const Staff* staff, const Fraction& tick)
{
    if (staff->isDrumStaff(tick)) {
        const Drumset* drumset = staff->part()->instrument(tick)->drumset();
        if (drumset) {
            return drumset->line(nval.pitch);
        }
    }

    if (nval.isRest()) {
        return staff->middleLine(tick);
    }

    int epitch = nval.pitch;
    int tpc = static_cast<int>(mu::engraving::Tpc::TPC_INVALID);
    noteValToEffectivePitchAndTpc(nval, staff, tick, epitch, tpc);

    return relStep(epitch, tpc, staff->clef(tick));
}

AccidentalVal noteValToAccidentalVal(const NoteVal& nval, const Staff* staff, const Fraction& tick)
{
    if (nval.isRest()) {
        return AccidentalVal::NATURAL;
    }

    if (staff->isDrumStaff(tick)) {
        return AccidentalVal::NATURAL;
    }

    int epitch = nval.pitch;
    int tpc = static_cast<int>(mu::engraving::Tpc::TPC_INVALID);
    noteValToEffectivePitchAndTpc(nval, staff, tick, epitch, tpc);

    return tpc2alter(tpc);
}

int compareNotesPos(const Note* n1, const Note* n2)
{
    if (n1->line() != n2->line() && !(n1->staffType()->isTabStaff())) {
        return n2->line() - n1->line();
    } else if (n1->string() != n2->string()) {
        return n2->string() - n1->string();
    } else {
        return n1->pitch() - n2->pitch();
    }
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

SymIdList timeSigSymIdsFromString(const String& string, TimeSigStyle timeSigStyle)
{
    static const std::map<Char, SymId> dict = {
        { '+',    SymId::timeSigPlusSmall },
        { '0',    SymId::timeSig0 },
        { '1',    SymId::timeSig1 },
        { '2',    SymId::timeSig2 },
        { '3',    SymId::timeSig3 },
        { '4',    SymId::timeSig4 },
        { '5',    SymId::timeSig5 },
        { '6',    SymId::timeSig6 },
        { '7',    SymId::timeSig7 },
        { '8',    SymId::timeSig8 },
        { '9',    SymId::timeSig9 },
        { 'C',    SymId::timeSigCommon },
        { '(',    SymId::timeSigParensLeftSmall },
        { ')',    SymId::timeSigParensRightSmall },
        { u'¢',   SymId::timeSigCutCommon },
        { u'½',   SymId::timeSigFractionHalf },
        { u'¼',   SymId::timeSigFractionQuarter },
        { '*',    SymId::timeSigMultiply },
        { 'X',    SymId::timeSigMultiply },
        { 'x',    SymId::timeSigMultiply },
        { u'×',   SymId::timeSigMultiply },
        { 59664,  SymId::mensuralProlation1 },
        { 79,     SymId::mensuralProlation2 },           // 'O'
        { 59665,  SymId::mensuralProlation2 },
        { 216,    SymId::mensuralProlation3 },           // 'Ø'
        { 59666,  SymId::mensuralProlation3 },
        { 59667,  SymId::mensuralProlation4 },
        { 59668,  SymId::mensuralProlation5 },
        { 59670,  SymId::mensuralProlation7 },
        { 59671,  SymId::mensuralProlation8 },
        { 59673,  SymId::mensuralProlation10 },
        { 59674,  SymId::mensuralProlation11 },
    };

    static const std::map<Char, SymId> dictLarge = {
        { 43,    SymId::timeSigPlusSmallLarge },             // '+'
        { 48,    SymId::timeSig0Large },                     // '0'
        { 49,    SymId::timeSig1Large },                     // '1'
        { 50,    SymId::timeSig2Large },                     // '2'
        { 51,    SymId::timeSig3Large },                     // '3'
        { 52,    SymId::timeSig4Large },                     // '4'
        { 53,    SymId::timeSig5Large },                     // '5'
        { 54,    SymId::timeSig6Large },                     // '6'
        { 55,    SymId::timeSig7Large },                     // '7'
        { 56,    SymId::timeSig8Large },                     // '8'
        { 57,    SymId::timeSig9Large },                     // '9'
        { 67,    SymId::timeSigCommonLarge },                // 'C'
        { 40,    SymId::timeSigParensLeftSmallLarge },       // '('
        { 41,    SymId::timeSigParensRightSmallLarge },      // ')'
        { 162,   SymId::timeSigCutCommonLarge },             // '¢'
        { 189,   SymId::timeSigFractionHalfLarge },          // '½'
        { 188,   SymId::timeSigFractionQuarterLarge },       // '¼'
        { 42,    SymId::timeSigMultiplyLarge },              // '*'
        { 88,    SymId::timeSigMultiplyLarge },              // 'X'
        { 120,   SymId::timeSigMultiplyLarge },              // 'x'
        { 215,   SymId::timeSigMultiplyLarge },              // '×'
    };

    static const std::map<Char, SymId> dictNarrow = {
        { 43,    SymId::timeSigPlusSmallNarrow },             // '+'
        { 48,    SymId::timeSig0Narrow },                     // '0'
        { 49,    SymId::timeSig1Narrow },                     // '1'
        { 50,    SymId::timeSig2Narrow },                     // '2'
        { 51,    SymId::timeSig3Narrow },                     // '3'
        { 52,    SymId::timeSig4Narrow },                     // '4'
        { 53,    SymId::timeSig5Narrow },                     // '5'
        { 54,    SymId::timeSig6Narrow },                     // '6'
        { 55,    SymId::timeSig7Narrow },                     // '7'
        { 56,    SymId::timeSig8Narrow },                     // '8'
        { 57,    SymId::timeSig9Narrow },                     // '9'
        { 67,    SymId::timeSigCommonNarrow },                // 'C'
        { 40,    SymId::timeSigParensLeftSmallNarrow },       // '('
        { 41,    SymId::timeSigParensRightSmallNarrow },      // ')'
        { 162,   SymId::timeSigCutCommonNarrow },             // '¢'
        { 189,   SymId::timeSigFractionHalfNarrow },          // '½'
        { 188,   SymId::timeSigFractionQuarterNarrow },       // '¼'
        { 42,    SymId::timeSigMultiplyNarrow },              // '*'
        { 88,    SymId::timeSigMultiplyNarrow },              // 'X'
        { 120,   SymId::timeSigMultiplyNarrow },              // 'x'
        { 215,   SymId::timeSigMultiplyNarrow },              // '×'
    };

    SymIdList list;
    for (size_t i = 0; i < string.size(); ++i) {
        SymId sym = muse::value(
            timeSigStyle == TimeSigStyle::NARROW ? dictNarrow
            : timeSigStyle == TimeSigStyle::LARGE ? dictLarge
            : dict,
            string.at(i), SymId::noSym);
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

double yStaffDifference(const System* system1, const System* system2, staff_idx_t staffIdx)
{
    if (!system1 || !system2) {
        return 0.0;
    }

    const SysStaff* staff1 = system1->staff(staffIdx);
    const SysStaff* staff2 = system2->staff(staffIdx);
    if (!staff1 || !staff2) {
        return 0.0;
    }
    return staff1->y() - staff2->y();
}

bool allowRemoveWhenRemovingStaves(EngravingItem* item, staff_idx_t startStaff, staff_idx_t endStaff)
{
    // Sanity checks
    if (!item || item->staffIdx() == muse::nidx || startStaff == muse::nidx || endStaff == muse::nidx) {
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
    if (!item || item->staffIdx() == muse::nidx || startStaff == muse::nidx || endStaff == muse::nidx) {
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
        } else if (e->isRest() && !toChordRest(e)->staffMove() && !toRest(e)->isFullMeasureRest()) {
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

            chords.push_back(chord);
        }
    }
}

std::vector<EngravingItem*> collectSystemObjects(const Score* score, const std::vector<Staff*>& staves)
{
    TRACEFUNC;

    std::vector<EngravingItem*> result;

    const TimeSigPlacement timeSigPlacement = score->style().styleV(Sid::timeSigPlacement).value<TimeSigPlacement>();
    const bool isOnStaffTimeSig = timeSigPlacement != TimeSigPlacement::NORMAL;

    for (const Measure* measure = score->firstMeasure(); measure; measure = measure->nextMeasure()) {
        for (EngravingItem* measureElement : measure->el()) {
            if (!measureElement || !measureElement->systemFlag() || measureElement->isLayoutBreak()) {
                continue;
            }
            if (!staves.empty()) {
                if (muse::contains(staves, measureElement->staff())) {
                    result.push_back(measureElement);
                }
            } else if (measureElement->isTopSystemObject()) {
                result.push_back(measureElement);
            }
        }

        for (const Segment& seg : measure->segments()) {
            if (seg.isType(Segment::CHORD_REST_OR_TIME_TICK_TYPE)) {
                for (EngravingItem* annotation : seg.annotations()) {
                    if (!annotation || !annotation->systemFlag()) {
                        continue;
                    }

                    if (!staves.empty()) {
                        if (muse::contains(staves, annotation->staff())) {
                            result.push_back(annotation);
                        }
                    } else if (annotation->isTopSystemObject()) {
                        result.push_back(annotation);
                    }
                }
            }

            if (isOnStaffTimeSig && seg.isType(SegmentType::TimeSigType)) {
                for (EngravingItem* item : seg.elist()) {
                    if (!item || !item->isTimeSig()) {
                        continue;
                    }

                    if (!staves.empty()) {
                        if (muse::contains(staves, item->staff())) {
                            result.push_back(item);
                        }
                    } else if (item->staffIdx() == 0) {
                        result.push_back(item);
                    }
                }
            }

            if (measure->repeatEnd() && seg.isType(SegmentType::BarLineType)) {
                for (EngravingItem* item : seg.elist()) {
                    if (!item || !item->isBarLine()) {
                        continue;
                    }

                    BarLine* bl = toBarLine(item);
                    if (Text* playCount = bl->playCountText()) {
                        result.push_back(playCount);
                    }
                }
            }
        }
    }

    for (const auto& pair : score->spanner()) {
        Spanner* spanner = pair.second;
        if (!spanner->systemFlag()) {
            continue;
        }

        if (!staves.empty()) {
            if (muse::contains(staves, spanner->staff())) {
                result.push_back(spanner);
            }
        } else if (spanner->isTopSystemObject()) {
            result.push_back(spanner);
        }
    }

    return result;
}

std::unordered_set<EngravingItem*> collectElementsAnchoredToChordRest(const ChordRest* cr)
{
    std::unordered_set<EngravingItem*> elems;
    for (EngravingItem* lyric : cr->lyrics()) {
        elems.emplace(lyric);
    }
    if (!cr->isChord()) {
        return elems;
    }
    const Chord* chord = toChord(cr);
    if (Arpeggio* arp = chord->arpeggio()) {
        elems.emplace(arp);
    }
    if (TremoloTwoChord* tremTwo = chord->tremoloTwoChord()) {
        elems.emplace(tremTwo);
    }
    if (TremoloSingleChord* tremSing = chord->tremoloSingleChord()) {
        elems.emplace(tremSing);
    }
    for (Articulation* art : chord->articulations()) {
        elems.emplace(art);
    }
    for (Chord* grace : chord->graceNotes()) {
        elems.emplace(grace);
    }
    return elems;
}

std::unordered_set<EngravingItem*> collectElementsAnchoredToNote(const Note* note, bool includeForwardTiesSpanners,
                                                                 bool includeBackwardTiesSpanners)
{
    std::unordered_set<EngravingItem*> elems;
    LaissezVib* lv = note->laissezVib();
    if (lv && !lv->segmentsEmpty()) {
        elems.emplace(lv);
    }
    PartialTie* ipt = note->incomingPartialTie();
    if (ipt && !ipt->segmentsEmpty()) {
        elems.emplace(ipt);
    }
    PartialTie* opt = note->outgoingPartialTie();
    if (opt && !opt->segmentsEmpty()) {
        elems.emplace(opt);
    }
    // The following is a bit of a hack - addressing properly would require a fingering rework...
    for (EngravingItem* elem : note->el()) {
        if (elem->isFingering()) {
            elems.emplace(elem);
        }
    }
    if (includeForwardTiesSpanners) {
        Tie* tieFor = note->tieFor();
        if (tieFor && !tieFor->segmentsEmpty()) {
            elems.emplace(tieFor);
        }
        for (Spanner* sp : note->spannerFor()) {
            if (sp->segmentsEmpty()) {
                continue;
            }
            elems.emplace(sp);
        }
    }
    if (includeBackwardTiesSpanners) {
        Tie* tieBack = note->tieBack();
        if (tieBack && !tieBack->segmentsEmpty()) {
            elems.emplace(tieBack);
        }
        for (Spanner* sp : note->spannerBack()) {
            if (sp->segmentsEmpty()) {
                continue;
            }
            elems.emplace(sp);
        }
    }
    return elems;
}

bool noteAnchoredSpannerIsInRange(const Spanner* spanner, const Fraction& rangeStart, const Fraction& rangeEnd)
{
    IF_ASSERT_FAILED(rangeStart < rangeEnd) {
        return false;
    }
    const EngravingItem* startElement = spanner->startElement();
    const EngravingItem* endElement = spanner->endElement();
    IF_ASSERT_FAILED(startElement && startElement->isNote() && endElement && endElement->isNote()) {
        LOGD() << "Cannot calculate isInRange - might be a partial tie or laissez vibrer";
        return false;
    }
    const Note* startNote = toNote(startElement);
    const Segment* startSeg = startNote->chord()->segment();
    if (startSeg->tick() < rangeStart) {
        return false;
    }
    const Note* endNote = toNote(endElement);
    const Segment* endSeg = endNote->chord()->segment();
    return !endSeg || endSeg->tick() <= rangeEnd;
}

String formatUniqueExcerptName(const String& baseName, const StringList& allExcerptLowerNames)
{
    String result = baseName;
    int num = 0;

    while (allExcerptLowerNames.contains(result.toLower())) {
        result = baseName + String(u" (%1)").arg(++num);
    }

    return result;
}

bool isFirstSystemKeySig(const KeySig* ks)
{
    if (!ks) {
        return false;
    }
    const System* sys = ks->measure()->system();
    if (!sys) {
        return false;
    }
    return ks->tick() == sys->firstMeasure()->tick();
}

String bendAmountToString(int fulls, int quarts)
{
    String string = (fulls != 0 || quarts == 0) ? String::number(fulls) : String();

    switch (quarts) {
    case 1:
        string += u"\u00BC";
        break;
    case 2:
        string += u"\u00BD";
        break;
    case 3:
        string += u"\u00BE";
        break;
    default:
        break;
    }

    return string;
}

InstrumentTrackId makeInstrumentTrackId(const EngravingItem* item)
{
    const Part* part = item->part();
    if (!part) {
        return InstrumentTrackId();
    }

    InstrumentTrackId trackId {
        part->id(),
        part->instrumentId(item->tick())
    };

    return trackId;
}

std::vector<Measure*> findFollowingRepeatMeasures(const Measure* measure)
{
    const MasterScore* master = measure->masterScore();
    const Score* score = measure->score();

    const Measure* masterMeasure = master->tick2measure(measure->tick());

    const RepeatList& repeatList = master->repeatList(true, false);

    std::vector<Measure*> measures;

    for (auto it = repeatList.begin(); it != repeatList.end(); it++) {
        const RepeatSegment* rs = *it;
        const auto nextSegIt = std::next(it);
        if (!rs->endsWithMeasure(masterMeasure) || nextSegIt == repeatList.end()) {
            continue;
        }

        // Get next segment
        const RepeatSegment* nextSeg = *nextSegIt;
        const Measure* firstMasterMeasure = nextSeg->firstMeasure();
        Measure* firstMeasure = firstMasterMeasure ? score->tick2measure(firstMasterMeasure->tick()) : nullptr;
        if (!firstMeasure) {
            continue;
        }

        measures.push_back(firstMeasure);
    }

    return measures;
}

std::vector<Measure*> findPreviousRepeatMeasures(const Measure* measure)
{
    const MasterScore* master = measure->masterScore();
    const Score* score = measure->score();

    const Measure* masterMeasure = master->tick2measure(measure->tick());

    const RepeatList& repeatList = master->repeatList(true, false);

    std::vector<Measure*> measures;

    for (auto it = repeatList.begin() + 1; it != repeatList.end(); it++) {
        const RepeatSegment* rs = *it;
        const auto prevSegIt = std::prev(it);
        if (!rs->startsWithMeasure(masterMeasure) || prevSegIt == repeatList.end()) {
            continue;
        }

        // Get next segment
        const RepeatSegment* prevSeg = *prevSegIt;
        const Measure* lastMasterMeasure = prevSeg->lastMeasure();
        Measure* lastMeasure = lastMasterMeasure ? score->tick2measure(lastMasterMeasure->tick()) : nullptr;
        if (!lastMeasure) {
            continue;
        }

        measures.push_back(lastMeasure);
    }

    return measures;
}

bool repeatHasPartialLyricLine(const Measure* endRepeatMeasure)
{
    const std::vector<Measure*> measures = findFollowingRepeatMeasures(endRepeatMeasure);
    const Score* score = endRepeatMeasure->score();

    for (const Measure* measure : measures) {
        const SpannerMap::IntervalList& spanners = score->spannerMap().findOverlapping(measure->tick().ticks(), measure->endTick().ticks());

        for (auto& spanner : spanners) {
            if (spanner.value->isPartialLyricsLine() && spanner.start == measure->tick().ticks()) {
                return true;
            }
        }
    }

    return false;
}

bool segmentsAreAdjacentInRepeatStructure(const Segment* firstSeg, const Segment* secondSeg)
{
    if (!firstSeg || !secondSeg) {
        return false;
    }
    const MasterScore* master = firstSeg->masterScore();

    Measure* firstMeasure = firstSeg->measure();
    Measure* secondMeasure = secondSeg->measure();

    if (firstMeasure == secondMeasure) {
        return true;
    }

    const Measure* firstMasterMeasure = master->tick2measure(firstMeasure->tick());
    const Measure* secondMasterMeasure = master->tick2measure(secondMeasure->tick());

    Score* score = firstSeg->score();

    const RepeatList& repeatList = score->repeatList(true, false);

    std::vector<const Measure*> measures;

    for (auto it = repeatList.begin(); it != repeatList.end(); it++) {
        const RepeatSegment* rs = *it;
        const auto nextSegIt = std::next(it);

        // Check if measures are in the same repeat segment
        if (rs->containsMeasure(firstMasterMeasure) && rs->containsMeasure(secondMasterMeasure)) {
            return true;
        }

        // Continue to build list of measures at the start of following repeat segments
        if (!rs->endsWithMeasure(firstMasterMeasure) || nextSegIt == repeatList.end()) {
            continue;
        }

        // Get next segment
        const RepeatSegment* nextSeg = *nextSegIt;
        const Measure* nextSegFirstMeasure = nextSeg->firstMeasure();
        if (!nextSegFirstMeasure) {
            continue;
        }

        measures.push_back(nextSegFirstMeasure);
    }

    // Check if second segment is in a following measure in the repeat structure
    for (const Measure* m : measures) {
        if (m == secondMasterMeasure) {
            return true;
        }
    }

    return false;
}

bool segmentsAreInDifferentRepeatSegments(const Segment* firstSeg, const Segment* secondSeg)
{
    if (!firstSeg || !secondSeg) {
        return false;
    }
    const MasterScore* master = firstSeg->masterScore();

    Measure* firstMeasure = firstSeg->measure();
    Measure* secondMeasure = secondSeg->measure();

    if (firstMeasure == secondMeasure) {
        return false;
    }

    const Measure* firstMasterMeasure = master->tick2measure(firstMeasure->tick());
    const Measure* secondMasterMeasure = master->tick2measure(secondMeasure->tick());

    Score* score = firstSeg->score();

    const RepeatList& repeatList = score->repeatList(true, false);

    std::vector<const Measure*> measures;

    for (auto it = repeatList.begin(); it != repeatList.end(); it++) {
        const RepeatSegment* rs = *it;

        if (!rs->containsMeasure(firstMasterMeasure) || !rs->containsMeasure(secondMasterMeasure)) {
            return true;
        }
    }

    return false;
}
}
