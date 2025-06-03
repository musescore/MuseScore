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

#include "chord.h"
#include "factory.h"
#include "harmony.h"
#include "key.h"
#include "keysig.h"
#include "linkedobjects.h"
#include "measure.h"
#include "note.h"
#include "part.h"
#include "pitchspelling.h"
#include "score.h"
#include "segment.h"
#include "select.h"
#include "staff.h"
#include "stafftype.h"
#include "undo.h"
#include "utils.h"

#include "log.h"

using namespace mu;
using namespace mu::engraving;

namespace mu::engraving {
//---------------------------------------------------------
//   keydiff2Interval
//    keysig -   -7(Cb) - +7(C#)
//---------------------------------------------------------

static Interval keydiff2Interval(Key oKey, Key nKey, TransposeDirection dir)
{
    static int stepTable[15] = {
        // C  G  D  A  E  B Fis
        0, 4, 1, 5, 2, 6, 3,
    };

    int cofSteps;       // circle of fifth steps
    int diatonic;
    if (nKey > oKey) {
        cofSteps = int(nKey) - int(oKey);
    } else {
        cofSteps = 12 - (int(oKey) - int(nKey));
    }
    diatonic = stepTable[(int(nKey) + 7) % 7] - stepTable[(int(oKey) + 7) % 7];
    if (diatonic < 0) {
        diatonic += 7;
    }
    diatonic %= 7;
    int chromatic = (cofSteps * 7) % 12;

    if ((dir == TransposeDirection::CLOSEST) && (chromatic > 6)) {
        dir = TransposeDirection::DOWN;
    }

    if (dir == TransposeDirection::DOWN) {
        chromatic = chromatic - 12;
        diatonic  = diatonic - 7;
        if (diatonic == -7) {
            diatonic = 0;
        }
        if (chromatic == -12) {
            chromatic = 0;
        }
    }
    return Interval(diatonic, chromatic);
}

/*!
 * Transposes both pitch and spelling for a note given an interval.
 *
 * Uses addition for pitch and transposeTpc() for spelling.
 *
 * @param pitch
 *  The initial (current) pitch. (pitch)
 * @param tpc
 *  The initial spelling. (tpc)
 * @param rpitch
 *  A pointer to the transposed pitch, calculated by this function. (pitch)
 * @param rtpc
 *  A pointer to the transposed spelling. (tcp)
 * @param interval
 *  The interval to transpose by.
 * @param useDoubleSharpsFlats
 *  Determines whether the output may include double sharps or flats (Abb)
 *  or should use an enharmonic pitch (Abb = G).
 */

void transposeInterval(int pitch, int tpc, int* rpitch, int* rtpc, Interval interval,
                       bool useDoubleSharpsFlats)
{
    *rpitch = pitch + interval.chromatic;
    *rtpc   = transposeTpc(tpc, interval, useDoubleSharpsFlats);
}

/*!
 * Transposes a pitch spelling given an interval.
 *
 * This function transposes a pitch spelling using first
 * a diatonic transposition and then calculating any accidentals.
 * This insures that the note is changed by the correct number of
 * scale degrees unless it would require too many accidentals.
 *
 * @param tpc
 *  The initial pitch spelling.
 * @param interval
 *  The interval to be transposed by.
 * @param useDoubleSharpsFlats
 *  Determines whether the output may include double sharps or flats (Abb)
 *  or should use an enharmonic pitch (Abb = G).
 *
 * @return
 *  The transposed pitch spelling (tpc).
 */

int transposeTpc(int tpc, Interval interval, bool useDoubleSharpsFlats)
{
    if (tpc == Tpc::TPC_INVALID) { // perfect unison & perfect octave
        return tpc;
    }

    int minAlter;
    int maxAlter;
    if (useDoubleSharpsFlats) {
        minAlter = -2;
        maxAlter = 2;
    } else {
        minAlter = -1;
        maxAlter = 1;
    }
    int steps     = interval.diatonic;
    int semitones = interval.chromatic;

// LOGD("transposeTpc tpc %d steps %d semitones %d", tpc, steps, semitones);
    if (semitones == 0 && steps == 0) {
        return tpc;
    }

    int step;
    int alter;
    int pitch = tpc2pitch(tpc);

    for (int k = 0; k < 10; ++k) {
        step = tpc2step(tpc) + steps;
        while (step < 0) {
            step += 7;
        }
        step   %= 7;
        int p1 = tpc2pitch(step2tpc(step, AccidentalVal::NATURAL));
        alter  = semitones - (p1 - pitch);
        // alter  = p1 + semitones - pitch;

//            if (alter < 0) {
//                  alter *= -1;
//                  alter = 12 - alter;
//                  }
        while (alter < 0) {
            alter += 12;
        }

        alter %= 12;
        if (alter > 6) {
            alter -= 12;
        }
        if (alter > maxAlter) {
            ++steps;
        } else if (alter < minAlter) {
            --steps;
        } else {
            break;
        }
//            LOGD("  again alter %d steps %d, step %d", alter, steps, step);
    }
//      LOGD("  = step %d alter %d  tpc %d", step, alter, step2tpc(step, alter));
    return step2tpc(step, AccidentalVal(alter));
}

//---------------------------------------------------------
//   transposeTpcDiatonicByKey
//
// returns the tpc diatonically transposed by steps, using degrees of given key
// option to keep any alteration tpc had with respect to unaltered corresponding degree of key
// option to enharmonically reduce tpc using double alterations
//---------------------------------------------------------

int transposeTpcDiatonicByKey(int tpc, int steps, Key key, bool keepAlteredDegrees, bool useDoubleSharpsFlats)
{
    if (tpc == Tpc::TPC_INVALID) {
        return tpc;
    }

    // get step for tpc with alteration for key
    int alter;
    int step = tpc2stepByKey(tpc, key, alter);

    // transpose step and get tpc for step/key
    step += steps;
    int newTpc = step2tpcByKey(step, key);

    // if required, apply alteration to new tpc
    if (keepAlteredDegrees) {
        newTpc += alter * TPC_DELTA_SEMITONE;
    }

    // check results are in ranges
    while (newTpc > Tpc::TPC_MAX) {
        newTpc   -= TPC_DELTA_ENHARMONIC;
    }
    while (newTpc < Tpc::TPC_MIN) {
        newTpc   += TPC_DELTA_ENHARMONIC;
    }

    // if required, reduce double alterations
    if (!useDoubleSharpsFlats) {
        if (newTpc >= Tpc::TPC_F_SS) {
            newTpc   -= TPC_DELTA_ENHARMONIC;
        }
        if (newTpc <= Tpc::TPC_B_BB) {
            newTpc   += TPC_DELTA_ENHARMONIC;
        }
    }

    return newTpc;
}

//---------------------------------------------------------
//   transpose
//    return false on failure
//---------------------------------------------------------

bool Score::transpose(Note* n, Interval interval, bool useDoubleSharpsFlats)
{
    int npitch;
    int ntpc1, ntpc2;
    transposeInterval(n->pitch(), n->tpc1(), &npitch, &ntpc1, interval, useDoubleSharpsFlats);
    if (n->transposition()) {
        if (n->staff()) {
            Interval v = n->staff()->transpose(n->tick());
            v.flip();
            ntpc2 = transposeTpc(ntpc1, v, useDoubleSharpsFlats);
        } else {
            int p;
            transposeInterval(n->pitch() - n->transposition(), n->tpc2(), &p, &ntpc2, interval, useDoubleSharpsFlats);
        }
    } else {
        ntpc2 = ntpc1;
    }
    if (npitch > 127) {
        return false;
    }
    undoChangePitch(n, npitch, ntpc1, ntpc2);
    return true;
}

//---------------------------------------------------------
//   transpose
//    return false on failure
//---------------------------------------------------------

bool Score::transpose(TransposeMode mode, TransposeDirection direction, Key trKey,
                      int transposeInterval, bool trKeys, bool transposeChordNames, bool useDoubleSharpsFlats)
{
    bool result = true;
    bool rangeSelection = selection().isRange();
    staff_idx_t startStaffIdx   = 0;
    staff_idx_t endStaffIdx     = 0;
    Fraction startTick  = Fraction(0, 1);
    if (rangeSelection) {
        startStaffIdx = selection().staffStart();
        endStaffIdx   = selection().staffEnd();
        startTick     = selection().tickStart();
    }

    Staff* st = staff(startStaffIdx);

    Interval interval;
    if (mode != TransposeMode::DIATONICALLY) {
        if (mode == TransposeMode::TO_KEY) {
            // calculate interval from "transpose to key"
            // find the key of the first pitched staff
            Key key = Key::C;
            for (staff_idx_t i = startStaffIdx; i < endStaffIdx; ++i) {
                Staff* s = staff(i);
                if (s->isPitchedStaff(startTick)) {
                    key = s->concertKey(startTick);
                    // remember this staff to use as basis in transposing key signatures
                    st = s;
                    break;
                }
            }
            if (key != trKey) {
                interval = keydiff2Interval(key, trKey, direction);
            } else {          //same key, which direction?
                if (direction == TransposeDirection::UP) {
                    interval = Interval(12);
                } else if (direction == TransposeDirection::DOWN) {
                    interval = Interval(-12);
                } else {      //don't do anything for same key and closest direction
                    return true;
                }
            }
        } else {
            interval = intervalList[transposeInterval];
            if (direction == TransposeDirection::DOWN) {
                interval.flip();
            }
        }

        if (!rangeSelection) {
            trKeys = false;
        }
        bool fullOctave = (interval.chromatic % 12) == 0;
        if (fullOctave && (mode != TransposeMode::TO_KEY)) {
            trKeys = false;
            transposeChordNames = false;
        }
    } else {  // diatonic transposition
        if (direction == TransposeDirection::DOWN) {
            transposeInterval *= -1;
        }
    }

    if (m_selection.isList()) {
        for (EngravingItem* e : m_selection.uniqueElements()) {
            if (!e->staff() || e->staff()->staffType(e->tick())->group() == StaffGroup::PERCUSSION) {
                continue;
            }
            if (e->staff()->primaryStaff() && e->staff()->primaryStaff()->staffType(Fraction(0, 1))->group() == StaffGroup::PERCUSSION) {
                continue;
            }
            if (e->isNote()) {
                Note* note = toNote(e);
                if (mode == TransposeMode::DIATONICALLY) {
                    note->transposeDiatonic(transposeInterval, trKeys, useDoubleSharpsFlats);
                } else {
                    if (!transpose(note, interval, useDoubleSharpsFlats)) {
                        result = false;
                        continue;
                    }
                }
            } else if (e->isHarmony() && transposeChordNames) {
                Harmony* h  = toHarmony(e);
                int rootTpc, baseTpc;
                Interval kv = e->staff()->transpose(e->tick());
                Interval iv = e->part()->instrument(e->tick())->transpose();
                Interval hInterval((interval.diatonic - kv.diatonic + iv.diatonic), (interval.chromatic - kv.chromatic + iv.chromatic));
                if (mode == TransposeMode::DIATONICALLY) {
                    Fraction tick = Fraction(0, 1);
                    if (h->explicitParent()->isSegment()) {
                        tick = toSegment(h->explicitParent())->tick();
                    } else if (h->explicitParent()->isFretDiagram() && h->explicitParent()->explicitParent()->isSegment()) {
                        tick = toSegment(h->explicitParent()->explicitParent())->tick();
                    }
                    Key key = !h->staff() ? Key::C : h->staff()->key(tick);
                    rootTpc = transposeTpcDiatonicByKey(h->rootTpc(),
                                                        transposeInterval, key, trKeys, useDoubleSharpsFlats);
                    baseTpc = transposeTpcDiatonicByKey(h->bassTpc(),
                                                        transposeInterval, key, trKeys, useDoubleSharpsFlats);
                } else {
                    rootTpc = transposeTpc(h->rootTpc(), hInterval, useDoubleSharpsFlats);
                    baseTpc = transposeTpc(h->bassTpc(), hInterval, useDoubleSharpsFlats);
                }
                undoTransposeHarmony(h, rootTpc, baseTpc);
            } else if (e->isKeySig() && mode != TransposeMode::DIATONICALLY && trKeys) {
                // TODO: this currently is disabled in dialog
                // if we enabled it, then it will need work
                // probably the code should look more like the range selection code
                KeySig* ks     = toKeySig(e);
                if (!ks->isAtonal()) {
                    Key key        = st->key(ks->tick());
                    Key cKey       = st->concertKey(ks->tick());
                    KeySigEvent ke = ks->keySigEvent();
                    ke.setConcertKey(cKey);
                    ke.setKey(key);
                    undo(new ChangeKeySig(ks, ke, ks->showCourtesy()));
                }
            }
        }
    }

    //--------------------------
    // process range selection
    //--------------------------

    std::list<Staff*> sl;
    for (staff_idx_t staffIdx = m_selection.staffStart(); staffIdx < m_selection.staffEnd(); ++staffIdx) {
        Staff* s = staff(staffIdx);
        if (s->staffType(Fraction(0, 1))->group() == StaffGroup::PERCUSSION) {        // ignore percussion staff
            continue;
        }
        if (s->primaryStaff() && s->primaryStaff()->staffType(Fraction(0, 1))->group() == StaffGroup::PERCUSSION) {
            continue;
        }
        if (muse::contains(sl, s)) {
            continue;
        }
        bool alreadyThere = false;
        for (Staff* s2 : sl) {
            if (s2 == s || (s2->links() && s2->links()->contains(s))) {
                alreadyThere = true;
                break;
            }
        }
        if (!alreadyThere) {
            sl.push_back(s);
        }
    }
    std::list<track_idx_t> tracks;
    for (Staff* s : sl) {
        track_idx_t idx = s->idx() * VOICES;
        for (voice_idx_t i = 0; i < VOICES; ++i) {
            if (selectionFilter().canSelectVoice(i)) {
                tracks.push_back(idx + i);
            }
        }
    }

    Segment* s1 = m_selection.startSegment();
    if (!s1) {
        return result;
    }
    // if range start on mmRest, get the actual segment instead
    if (s1->measure()->isMMRest()) {
        s1 = tick2segment(s1->tick(), true, s1->segmentType(), false);
    }
    // if range starts with first CR of measure
    // then start looping from very beginning of measure
    // so we include key signature and can transpose that if requested
    if (s1->rtick().isZero()) {
        s1 = s1->measure()->first();
    }
    Segment* s2 = m_selection.endSegment();
    for (Segment* segment = s1; segment && segment != s2; segment = segment->next1()) {
        if (!segment->enabled()) {
            continue;
        }
        for (track_idx_t track : tracks) {
            if (staff(track / VOICES)->staffType(s1->tick())->group() == StaffGroup::PERCUSSION) {
                continue;
            }
            EngravingItem* e = segment->element(track);
            if (!e) {
                continue;
            }

            if (e->isChord()) {
                Chord* chord = toChord(e);
                const std::vector<Note*> nl = chord->notes();
                for (size_t noteIdx = 0; noteIdx < nl.size(); ++noteIdx) {
                    if (!m_selectionFilter.canSelectNoteIdx(noteIdx, nl.size(), m_selection.rangeContainsMultiNoteChords())) {
                        continue;
                    }
                    Note* note = nl.at(noteIdx);
                    if (mode == TransposeMode::DIATONICALLY) {
                        note->transposeDiatonic(transposeInterval, trKeys, useDoubleSharpsFlats);
                        continue;
                    }
                    if (!transpose(note, interval, useDoubleSharpsFlats)) {
                        result = false;
                    }
                }
                for (Chord* g : chord->graceNotes()) {
                    for (Note* n : g->notes()) {
                        if (mode == TransposeMode::DIATONICALLY) {
                            n->transposeDiatonic(transposeInterval, trKeys, useDoubleSharpsFlats);
                        } else {
                            if (!transpose(n, interval, useDoubleSharpsFlats)) {
                                result = false;
                                continue;
                            }
                        }
                    }
                }
            } else if (e->isKeySig() && trKeys && mode != TransposeMode::DIATONICALLY) {
                KeySig* ks = toKeySig(e);
                Fraction tick = segment->tick();
                bool startKey = tick == s1->tick() && !isFirstSystemKeySig(ks);
                bool addKey = ks->isChange();
                if ((startKey || addKey) && !ks->isAtonal()) {
                    Staff* staff = ks->staff();
                    Key oKey = ks->concertKey();
                    Key nKey = transposeKey(oKey, interval);
                    KeySigEvent ke = ks->keySigEvent();
                    // don't transpose instrument change KS, was automagically translated when precedent KS was changed
                    if (!ke.forInstrumentChange() || startKey) {
                        ke.setConcertKey(nKey);
                    }
                    // undoChangeKey handles linked staves/parts and generating new keysigs as needed
                    // it always sets the keysig non-generated
                    // so only call it when needed
                    undoChangeKeySig(staff, tick, ke);
                }
            }
        }
        if (transposeChordNames && selectionFilter().isFiltered(ElementsSelectionFilterTypes::CHORD_SYMBOL)) {
            for (EngravingItem* e : segment->annotations()) {
                if (!e->isHarmony() || (!muse::contains(tracks, e->track()))) {
                    continue;
                }
                // TODO also source interval should reflect modified key (f.ex. by prefer flat)
                // now we calculate interval from first pitched staff
                // but even if it were right staff, this may differ each tick (actual key signature)
                // we can  add something like "concert pitch rootTpc" to harmony definition, or ...
                Interval kv = e->staff()->transpose(e->tick());
                Interval iv = e->part()->instrument(e->tick())->transpose();
                Interval hInterval((interval.diatonic - kv.diatonic + iv.diatonic), (interval.chromatic - kv.chromatic + iv.chromatic));

                Harmony* hh  = toHarmony(e);
                int rootTpc, baseTpc;
                // undoTransposeHarmony does not do links
                // because it is also used to handle transposing instruments
                // and score / parts could be in different concert pitch states
                for (EngravingObject* se : hh->linkList()) {
                    Harmony* h = toHarmony(se);
                    if (mode == TransposeMode::DIATONICALLY) {
                        Fraction tick = segment->tick();
                        Key key = !h->staff() ? Key::C : h->staff()->key(tick);
                        rootTpc = transposeTpcDiatonicByKey(h->rootTpc(),
                                                            transposeInterval, key, trKeys, useDoubleSharpsFlats);
                        baseTpc = transposeTpcDiatonicByKey(h->bassTpc(),
                                                            transposeInterval, key, trKeys, useDoubleSharpsFlats);
                    } else {
                        rootTpc = transposeTpc(h->rootTpc(), hInterval, useDoubleSharpsFlats);
                        baseTpc = transposeTpc(h->bassTpc(), hInterval, useDoubleSharpsFlats);
                    }
                    undoTransposeHarmony(h, rootTpc, baseTpc);
                }
            }
        }
    }
    //
    // create missing key signatures
    //
    if (trKeys && (mode != TransposeMode::DIATONICALLY) && (s1->tick() == Fraction(0, 1))) {
        for (track_idx_t track : tracks) {
            if (track % VOICES) {
                continue;
            }
            Segment* seg = firstMeasure()->undoGetSegmentR(SegmentType::KeySig, Fraction(0, 1));
            KeySig* ks = toKeySig(seg->element(track));
            if (!ks) {
                ks = Factory::createKeySig(seg);
                ks->setTrack(track);
                Key nKey = transposeKey(Key::C, interval);
                ks->setKey(nKey);
                ks->setParent(seg);
                undoAddElement(ks);
            }
        }
    }
    return result;
}

//---------------------------------------------------------
//   transposeKeys
//    key -   -7(Cb) - +7(C#)
//---------------------------------------------------------

void Score::transposeKeys(staff_idx_t staffStart, staff_idx_t staffEnd, const Fraction& ts, const Fraction& tickEnd,
                          bool flip)
{
    Fraction tickStart(ts);
    if (tickStart < Fraction(0, 1)) {            // -1 and 0 are valid values to indicate start of score
        tickStart = Fraction(0, 1);
    }
    for (staff_idx_t staffIdx = staffStart; staffIdx < staffEnd; ++staffIdx) {
        Staff* st = staff(staffIdx);
        if (st->staffType(tickStart)->group() == StaffGroup::PERCUSSION) {
            continue;
        }

        bool createKey = tickStart.isZero();
        for (Segment* s = firstSegment(SegmentType::KeySig); s; s = s->next1(SegmentType::KeySig)) {
            if (!s->enabled() || s->tick() < tickStart) {
                continue;
            }
            if (tickEnd != Fraction(-1, 1) && s->tick() >= tickEnd) {
                break;
            }
            KeySig* ks = toKeySig(s->element(staffIdx * VOICES));
            if (!ks || ks->generated()) {
                continue;
            }
            if (!ks->isAtonal()) {
                Interval v = st->part()->instrument(s->tick())->transpose();
                KeySigEvent ke = st->keySigEvent(s->tick());
                PreferSharpFlat pref = ks->part()->preferSharpFlat();
                Key nKey = ke.concertKey();
                if (flip && !v.isZero()) {
                    v.flip();
                    nKey = transposeKey(ke.concertKey(), v, pref);
                }

                ke.setKey(nKey);
                undo(new ChangeKeySig(ks, ke, ks->showCourtesy()));
            }
            if (s->tick().isZero()) {
                createKey = false;
            }
        }
        if (createKey && firstMeasure()) {
            Segment* seg = firstMeasure()->undoGetSegmentR(SegmentType::KeySig, Fraction(0, 1));
            seg->setHeader(true);
            KeySig* ks = Factory::createKeySig(seg);
            ks->setTrack(staffIdx * VOICES);
            Interval v = ks->part()->instrument()->transpose();
            Key cKey = Key::C;
            Key nKey = cKey;
            if (flip) {
                if (!v.isZero()) {
                    v.flip();
                    nKey = transposeKey(Key::C, v, ks->part()->preferSharpFlat());
                }
            } else {
                cKey = transposeKey(Key::C, v);
                nKey = cKey;
            }
            ks->setKey(cKey, nKey);
            ks->setParent(seg);
            undoAddElement(ks);
        }
    }
}

//---------------------------------------------------------
//   transposeSemitone
//---------------------------------------------------------

void Score::transposeSemitone(int step)
{
    if (step == 0) {
        return;
    }
    if (step > 1) {
        step = 1;
    }
    if (step < -1) {
        step = -1;
    }

    TransposeDirection dir = step > 0 ? TransposeDirection::UP : TransposeDirection::DOWN;

    int keyType = int(staff(0)->key(Fraction(0, 1))) + 7;     // ??

    int intervalListArray[15][2] = {
        // up - down
        { 1, 1 },      // Cb
        { 1, 1 },      // Gb
        { 1, 1 },      // Db
        { 1, 1 },      // Ab
        { 1, 1 },      // Eb
        { 1, 1 },      // Bb
        { 1, 1 },      // F
        { 1, 1 },      // C
        { 1, 1 },      // G
        { 1, 1 },      // D
        { 1, 1 },      // A
        { 1, 1 },      // E
        { 1, 1 },      // B
        { 1, 1 },      // F#
        { 1, 1 }       // C#
    };

    const int interval = intervalListArray[keyType][step > 0 ? 0 : 1];

    if (!transpose(TransposeMode::BY_INTERVAL, dir, Key::C, interval, true, true, false)) {
        LOGD("Score::transposeSemitone: failed");
        // TODO: set error message
    } else {
        setSelectionChanged(true);
    }
}

//---------------------------------------------------------
//   Note::transposeDiatonic
//---------------------------------------------------------

void Note::transposeDiatonic(int interval, bool keepAlterations, bool useDoubleAccidentals)
{
    // compute note current absolute step
    int alter;
    Fraction tick = chord()->segment()->tick();
    Key key       = staff() ? staff()->key(tick) : Key::C;
    int absStep   = pitch2absStepByKey(epitch(), tpc(), key, alter);

    // get pitch and tcp corresponding to unaltered degree for this key
    int newPitch = absStep2pitchByKey(absStep + interval, key);
    int newTpc   = step2tpcByKey((absStep + interval) % STEP_DELTA_OCTAVE, key);

    // if required, transfer original degree alteration to new pitch and tpc
    if (keepAlterations) {
        newPitch += alter;
        newTpc  += alter * TPC_DELTA_SEMITONE;
    }

    // transpose appropriately
    int newTpc1 = TPC_INVALID;
    int newTpc2 = TPC_INVALID;
    Interval v   = staff() ? staff()->transpose(tick) : Interval(0);
    if (concertPitch()) {
        v.flip();
        newTpc1 = newTpc;
        newTpc2 = mu::engraving::transposeTpc(newTpc, v, true);
    } else {
        newPitch += v.chromatic;
        newTpc1 = mu::engraving::transposeTpc(newTpc, v, true);
        newTpc2 = newTpc;
    }

    // check results are in ranges
    while (newPitch > 127) {
        newPitch -= PITCH_DELTA_OCTAVE;
    }
    while (newPitch < 0) {
        newPitch += PITCH_DELTA_OCTAVE;
    }
    while (newTpc1 > Tpc::TPC_MAX) {
        newTpc1 -= TPC_DELTA_ENHARMONIC;
    }
    while (newTpc1 < Tpc::TPC_MIN) {
        newTpc1 += TPC_DELTA_ENHARMONIC;
    }
    while (newTpc2 > Tpc::TPC_MAX) {
        newTpc2 -= TPC_DELTA_ENHARMONIC;
    }
    while (newTpc2 < Tpc::TPC_MIN) {
        newTpc2 += TPC_DELTA_ENHARMONIC;
    }

    // if required, reduce double alterations
    if (!useDoubleAccidentals) {
        if (newTpc1 >= Tpc::TPC_F_SS) {
            newTpc1 -= TPC_DELTA_ENHARMONIC;
        }
        if (newTpc1 <= Tpc::TPC_B_BB) {
            newTpc1 += TPC_DELTA_ENHARMONIC;
        }
        if (newTpc2 >= Tpc::TPC_F_SS) {
            newTpc2 -= TPC_DELTA_ENHARMONIC;
        }
        if (newTpc2 <= Tpc::TPC_B_BB) {
            newTpc2 += TPC_DELTA_ENHARMONIC;
        }
    }

    // store new data
    score()->undoChangePitch(this, newPitch, newTpc1, newTpc2);
}

//---------------------------------------------------------
//   transposeDiatonicAlterations
//---------------------------------------------------------

void Score::transposeDiatonicAlterations(TransposeDirection direction)
{
    // Transpose current selection diatonically (up/down) while keeping degree alterations
    // Note: Score::transpose() absolutely requires valid selection before invocation.
    if (!selection().isNone()) {
        transpose(TransposeMode::DIATONICALLY, direction, Key::C, 1, true, true, true);
        setPlayNote(true);     // For when selection is a single note, also playback that note
        setSelectionChanged(true);     // This will update the on-screen keyboard
    }
}

//---------------------------------------------------------
//   transpositionChanged
//---------------------------------------------------------

void Score::transpositionChanged(Part* part, Interval oldV, Fraction tickStart, Fraction tickEnd)
{
    if (tickStart == Fraction(-1, 1)) {
        tickStart = Fraction(0, 1);
    }

    // transpose keys first
    std::set<Score*> scores;
    for (Staff* ls : part->staff(0)->staffList()) {
        // TODO: special handling for linked staves within a score
        // could be useful for capo
        Score* score = ls->score();
        if (muse::contains(scores, score)) {
            continue;
        }
        scores.insert(score);
        Part* lp = ls->part();
        if (!score->style().styleB(Sid::concertPitch)) {
            score->transposeKeys(lp->startTrack() / VOICES, lp->endTrack() / VOICES, tickStart, tickEnd, true);
        }
    }

    // now transpose notes and chord symbols
    for (Segment* s = firstSegment(Segment::CHORD_REST_OR_TIME_TICK_TYPE); s; s = s->next1(Segment::CHORD_REST_OR_TIME_TICK_TYPE)) {
        if (s->tick() < tickStart) {
            continue;
        }
        if (tickEnd != Fraction(-1, 1) && s->tick() >= tickEnd) {
            break;
        }
        Interval v = part->staff(0)->transpose(s->tick());
        v.flip();
        Interval diffV(oldV.diatonic + v.diatonic, oldV.chromatic + v.chromatic);
        for (Staff* st : part->staves()) {
            if (st->staffType(tickStart)->group() == StaffGroup::PERCUSSION) {
                continue;
            }
            track_idx_t t1 = st->idx() * VOICES;
            track_idx_t t2 = t1 + VOICES;
            for (track_idx_t track = t1; track < t2; ++track) {
                EngravingItem* e = s->element(track);
                if (e && e->isChord()) {
                    Chord* c = toChord(e);
                    for (Chord* gc : c->graceNotes()) {
                        for (Note* n : gc->notes()) {
                            int tpc = transposeTpc(n->tpc1(), v, true);
                            n->undoChangeProperty(Pid::TPC2, tpc);
                        }
                    }
                    for (Note* n : c->notes()) {
                        int tpc = transposeTpc(n->tpc1(), v, true);
                        n->undoChangeProperty(Pid::TPC2, tpc);
                    }
                }
                // find chord symbols
                for (EngravingItem* element : s->annotations()) {
                    if (element->track() != track || element->type() != ElementType::HARMONY) {
                        continue;
                    }
                    Harmony* h  = toHarmony(element);
                    int rootTpc = transposeTpc(h->rootTpc(), diffV, false);
                    int baseTpc = transposeTpc(h->bassTpc(), diffV, false);
                    for (EngravingObject* scoreElement : h->linkList()) {
                        if (!scoreElement->style().styleB(Sid::concertPitch)) {
                            undoTransposeHarmony(toHarmony(scoreElement), rootTpc, baseTpc);
                        }
                    }
                }
            }
        }
    }
}

void Score::transpositionChanged(Part* part, const Fraction& instrumentTick, Interval oldTransposition)
{
    Fraction tickStart = instrumentTick;
    Fraction tickEnd = { -1, 1 };

    auto mainInstrumentEndIt = part->instruments().upper_bound(tickStart.ticks());
    if (mainInstrumentEndIt != part->instruments().cend()) {
        tickEnd = Fraction::fromTicks(mainInstrumentEndIt->first);
    }

    transpositionChanged(part, oldTransposition, tickStart, tickEnd);
}
}
