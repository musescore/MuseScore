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

#include "actionicon.h"
#include "accidental.h"
#include "arpeggio.h"
#include "chord.h"
#include "chordrest.h"
#include "clef.h"
#include "fret.h"
#include "harmony.h"
#include "laissezvib.h"
#include "lyrics.h"
#include "marker.h"
#include "masterscore.h"
#include "repeatlist.h"
#include "keysig.h"
#include "measure.h"
#include "measurenumber.h"
#include "measurerepeat.h"
#include "note.h"
#include "page.h"
#include "part.h"
#include "playcounttext.h"
#include "partialtie.h"
#include "pitchspelling.h"
#include "rest.h"
#include "score.h"
#include "segment.h"
#include "select.h"
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
    int l = std::max(ClefInfo::pitchOffset(clef) - line, 0);
    int octave = l / STEP_DELTA_OCTAVE;
    l %= STEP_DELTA_OCTAVE;

    int pitch = pitchKeyAdjust(l, key) + octave * PITCH_DELTA_OCTAVE;
    return clampPitch(pitch);
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
    if (!pitchIsValid(v)) {
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
        return INVALID_PITCH;
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

    if (!nextSegment) {
        nextSegment = seg->next1(SegmentType::ChordRest);
        while (nextSegment && nextSegment->tick() < chord->endTick()) {
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
            Chord* gc = gna.front();
            note2 = gc->findNote(note->pitch());
            if (note2) {
                return note2;
            }
        }
    }
    // at this point, chord is a regular chord, not a grace chord
    // and we are looking for a note in the *next* chord (grace or regular)

    int idx1 = note->unisonIndex();
    Part* part = chord->part();
    for (track_idx_t track = part->startTrack(); track < part->endTrack(); ++track) {
        EngravingItem* e = nextSegment->element(track);
        if (!e || !e->isChord()) {
            continue;
        }
        Chord* c = toChord(e);
        if (c->vStaffIdx() != chord->vStaffIdx()) {
            // this check is needed as we are iterating over all staves to capture cross-staff chords
            continue;
        }
        // if there are grace notes before, try to tie to first one
        std::vector<Chord*> gnb = c->graceNotesBefore();
        if (!gnb.empty()) {
            Chord* gc = gnb.front();
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
    int line = tpc2step(tpc) + (pitch / PITCH_DELTA_OCTAVE) * STEP_DELTA_OCTAVE;
    int tpcPitch = tpc2pitch(tpc);

    if (tpcPitch < MIN_PITCH) {
        line += STEP_DELTA_OCTAVE;
    } else {
        line -= (tpcPitch / PITCH_DELTA_OCTAVE) * STEP_DELTA_OCTAVE;
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
    while ((p = line2pitch(lineR2, clefR, Key::C)) > goalpitch && p < MAX_PITCH) {
        lineR2++;
    }
    while ((p = line2pitch(lineR2, clefR, Key::C)) < goalpitch && p > MIN_PITCH) {
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
        { '[',    SymId::timeSigBracketLeftSmall },
        { ']',    SymId::timeSigBracketRightSmall },
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
        { '+',    SymId::timeSigPlusSmallLarge },
        { '0',    SymId::timeSig0Large },
        { '1',    SymId::timeSig1Large },
        { '2',    SymId::timeSig2Large },
        { '3',    SymId::timeSig3Large },
        { '4',    SymId::timeSig4Large },
        { '5',    SymId::timeSig5Large },
        { '6',    SymId::timeSig6Large },
        { '7',    SymId::timeSig7Large },
        { '8',    SymId::timeSig8Large },
        { '9',    SymId::timeSig9Large },
        { 'C',    SymId::timeSigCommonLarge },
        { '(',    SymId::timeSigParensLeftSmallLarge },
        { ')',    SymId::timeSigParensRightSmallLarge },
        { '[',    SymId::timeSigBracketLeftSmallLarge },
        { ']',    SymId::timeSigBracketRightSmallLarge },
        { u'¢',   SymId::timeSigCutCommonLarge },
        { u'½',   SymId::timeSigFractionHalfLarge },
        { u'¼',   SymId::timeSigFractionQuarterLarge },
        { '*',    SymId::timeSigMultiplyLarge },
        { 'X',    SymId::timeSigMultiplyLarge },
        { 'x',    SymId::timeSigMultiplyLarge },
        { u'×',   SymId::timeSigMultiplyLarge },
    };

    static const std::map<Char, SymId> dictNarrow = {
        { '+',    SymId::timeSigPlusSmallNarrow },
        { '0',    SymId::timeSig0Narrow },
        { '1',    SymId::timeSig1Narrow },
        { '2',    SymId::timeSig2Narrow },
        { '3',    SymId::timeSig3Narrow },
        { '4',    SymId::timeSig4Narrow },
        { '5',    SymId::timeSig5Narrow },
        { '6',    SymId::timeSig6Narrow },
        { '7',    SymId::timeSig7Narrow },
        { '8',    SymId::timeSig8Narrow },
        { '9',    SymId::timeSig9Narrow },
        { 'C',    SymId::timeSigCommonNarrow },
        { '(',    SymId::timeSigParensLeftSmallNarrow },
        { ')',    SymId::timeSigParensRightSmallNarrow },
        { '[',    SymId::timeSigBracketLeftSmallNarrow },
        { ']',    SymId::timeSigBracketRightSmallNarrow },
        { u'¢',   SymId::timeSigCutCommonNarrow },
        { u'½',   SymId::timeSigFractionHalfNarrow },
        { u'¼',   SymId::timeSigFractionQuarterNarrow },
        { '*',    SymId::timeSigMultiplyNarrow },
        { 'X',    SymId::timeSigMultiplyNarrow },
        { 'X',    SymId::timeSigMultiplyNarrow },
        { u'×',   SymId::timeSigMultiplyNarrow },
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

bool dragPositionToMeasure(const PointF& pos, const Score* score,
                           Measure** measure, staff_idx_t* staffIdx,
                           const double spacingFactor)
{
    const System* preferredSystem = (*measure) ? (*measure)->system() : nullptr;

    Measure* m = score->searchMeasure(pos, preferredSystem, spacingFactor);
    if (!m) {
        return false;
    }

    const System* system = m->system();
    const double y = pos.y() - system->canvasPos().y();
    const staff_idx_t i = system->searchStaff(y, *staffIdx, spacingFactor);
    if (!score->staff(i)) {
        return false;
    }

    *measure = m;
    *staffIdx = i;
    return true;
}

bool dragPositionToSegment(const PointF& pos, const Measure* measure, const staff_idx_t staffIdx,
                           Segment** segment,
                           const double spacingFactor, const bool allowTimeAnchor)
{
    const track_idx_t strack = staffIdx * VOICES;
    const track_idx_t etrack = strack + VOICES;

    const double x = pos.x() - measure->canvasPos().x();
    const SegmentType st = allowTimeAnchor ? Segment::CHORD_REST_OR_TIME_TICK_TYPE : SegmentType::ChordRest;
    Segment* s = measure->searchSegment(x, st, strack, etrack, *segment, spacingFactor);
    if (!s) {
        return false;
    }

    *segment = segmentOrChordRestSegmentAtSameTick(s);
    return true;
}

Segment* segmentOrChordRestSegmentAtSameTick(Segment* segment)
{
    IF_ASSERT_FAILED(segment) {
        return nullptr;
    }

    // If TimeTick and ChordRest segments are at the same tick, prefer ChordRest
    if (segment->isTimeTickType() && segment->measure()) {
        if (Segment* crSegAtSameTick = segment->measure()->findSegmentR(SegmentType::ChordRest, segment->rtick())) {
            return crSegAtSameTick;
        }
    }

    return segment;
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
    bool nextRemainingIsSystemObjectStaff = nextRemaining && nextRemaining->isSystemObjectStaff();
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
    bool nextAfterInsertedIsSystemObjectStaff = nextAfterInserted && nextAfterInserted->isSystemObjectStaff();
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
            if (seg.isType(Segment::CHORD_REST_OR_TIME_TICK_TYPE | SegmentType::EndBarLine)) {
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

bool isValidBarLineForRepeatSection(const Segment* firstSeg, const Segment* secondSeg)
{
    if (!firstSeg || !secondSeg) {
        return false;
    }
    if (!firstSeg->isType(SegmentType::BarLineType)) {
        return false;
    }

    const MasterScore* master = firstSeg->masterScore();

    Measure* firstMeasure = firstSeg->measure();
    Measure* secondMeasure = secondSeg->measure();

    const Measure* firstMasterMeasure = master->tick2measure(firstMeasure->tick());
    const Measure* secondMasterMeasure = master->tick2measure(secondMeasure->tick());
    const Measure* adjacentMasterMeasure = firstMasterMeasure->nextMeasure();

    Score* score = firstSeg->score();

    const RepeatList& repeatList = score->repeatList(true, false);

    std::vector<const Measure*> measures;

    bool segEndsWithBl = false;
    bool adjacentAndSecondShareSegment = false;

    for (auto it = repeatList.begin(); it != repeatList.end(); it++) {
        const RepeatSegment* rs = *it;

        if (rs->endsWithMeasure(firstMasterMeasure)) {
            segEndsWithBl = true;
        }

        if (rs->startsWithMeasure(adjacentMasterMeasure) && rs->containsMeasure(secondMasterMeasure)) {
            adjacentAndSecondShareSegment = true;
        }
    }

    return segEndsWithBl && adjacentAndSecondShareSegment;
}

MeasureBeat findBeat(const Score* score, int tick)
{
    MeasureBeat measureBeat;
    if (!score || !score->checkHasMeasures()) {
        return measureBeat;
    }

    int ticks = 0;
    int beatIndex = 0;
    score->sigmap()->tickValues(tick, &measureBeat.measureIndex, &beatIndex, &ticks);

    const TimeSigFrac timeSig = score->sigmap()->timesig(Fraction::fromTicks(tick)).timesig();
    const int ticksB = ticks_beat(timeSig.denominator());

    measureBeat.beat = beatIndex + ticks / static_cast<float>(ticksB);
    measureBeat.maxMeasureIndex = const_cast<Score*>(score)->measures()->size() - 1;
    measureBeat.maxBeatIndex = timeSig.numerator() - 1;

    return measureBeat;
}

bool isElementInFretBox(const EngravingItem* item)
{
    if (item->isHarmony()) {
        return toHarmony(item)->isInFretBox();
    } else if (item->isFretDiagram()) {
        return toFretDiagram(item)->isInFretBox();
    }
    return false;
}

std::vector<EngravingItem*> filterTargetElements(const Selection& sel, EngravingItem* dropElement, bool& unique)
{
    bool uniqueMeasures =  false;
    bool uniqueStaves = false;

    switch (dropElement->type()) {
    // Brackets have a special logic for range selections.
    // For other selections add only one to each staff:
    case ElementType::BRACKET:
        if (sel.isRange()) {
            unique = true;
            return { sel.startSegment()->firstElementForNavigation(sel.staffStart()) };
        }
        uniqueStaves = true;
        break;

    // Barlines are only added to measures in range selections:
    case ElementType::BAR_LINE:
        uniqueMeasures = sel.isRange();
        break;

    // For multi-measure measure repeats, avoid overlap by adding only one per staff:
    case ElementType::MEASURE_REPEAT:
        uniqueStaves = true;
        uniqueMeasures = toMeasureRepeat(dropElement)->numMeasures() == 1;
        break;

    // Add these elements only once per staff in range selections, else once per staff per measure:
    case ElementType::SPACER:
    case ElementType::STAFFTYPE_CHANGE:
        uniqueStaves = true;
        [[fallthrough]];

    // Add these elements once per measure in range selections, else once total:
    case ElementType::MARKER:
    case ElementType::JUMP:
    case ElementType::VBOX:
    case ElementType::HBOX:
    case ElementType::TBOX:
    case ElementType::FBOX:
    case ElementType::MEASURE:
        uniqueMeasures = !sel.isRange();
        break;

    // Add these elements once per measure, always:
    case ElementType::MEASURE_NUMBER:
        uniqueMeasures = true;
        break;

    case ElementType::ACTION_ICON: {
        const ActionIconType actionType = toActionIcon(dropElement)->actionType();
        switch (actionType) {
        case ActionIconType::STAFF_TYPE_CHANGE:
            uniqueStaves = true;
            [[fallthrough]];
        case ActionIconType::VFRAME:
        case ActionIconType::HFRAME:
        case ActionIconType::TFRAME:
        case ActionIconType::FFRAME:
        case ActionIconType::MEASURE:
            uniqueMeasures = !sel.isRange();
            break;
        default: break;
        }
        break;
    }
    default: break;
    }

    unique = uniqueStaves || uniqueMeasures;
    if (!unique) {
        return sel.elements();
    }

    std::vector<EngravingItem*> result;
    if (uniqueStaves && uniqueMeasures) {
        std::vector<MStaff*> foundMStaves;
        for (EngravingItem* e : sel.elements()) {
            if (Measure* m = e->findMeasure()) {
                if (!muse::contains(foundMStaves, m->mstaves().at(e->staffIdx()))) {
                    result.emplace_back(e);
                    foundMStaves.emplace_back(m->mstaves().at(e->staffIdx()));
                }
            }
        }
    } else if (uniqueStaves) {
        std::vector<staff_idx_t> foundStaves;
        for (EngravingItem* e : sel.elements()) {
            if (!muse::contains(foundStaves, e->staffIdx())) {
                result.emplace_back(e);
                foundStaves.emplace_back(e->staffIdx());
            }
        }
    } else {
        std::vector<MeasureBase*> foundMeasures;
        for (EngravingItem* e : sel.elements()) {
            if (MeasureBase* mb = e->findMeasureBase()) {
                if (!muse::contains(foundMeasures, mb)) {
                    result.emplace_back(e);
                    foundMeasures.emplace_back(mb);
                }
            }
        }
    }
    return result;
}
}
