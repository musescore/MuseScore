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

#include "transpose.h"

#include "../dom/harmony.h"
#include "../dom/part.h"
#include "../dom/segment.h"
#include "../dom/staff.h"
#include "../dom/utils.h"
#include "../dom/note.h"
#include "../dom/factory.h"
#include "../dom/keysig.h"
#include "../dom/linkedobjects.h"
#include "../dom/utils.h"
#include "editing/editkeysig.h"

using namespace mu::engraving;

bool Transpose::transpose(Score* score, TransposeMode mode, TransposeDirection direction, Key trKey, int transposeInterval, bool trKeys,
                          bool transposeChordNames, bool useDoubleSharpsFlats)
{
    const Selection& selection = score->selection();
    bool result = true;
    bool rangeSelection = selection.isRange();
    staff_idx_t startStaffIdx   = 0;
    staff_idx_t endStaffIdx     = 0;
    Fraction startTick  = Fraction(0, 1);
    if (rangeSelection) {
        startStaffIdx = selection.staffStart();
        endStaffIdx   = selection.staffEnd();
        startTick     = selection.tickStart();
    }

    Staff* st = score->staff(startStaffIdx);

    Interval interval;
    if (mode != TransposeMode::DIATONICALLY) {
        if (mode == TransposeMode::TO_KEY) {
            // calculate interval from "transpose to key"
            // find the key of the first pitched staff
            Key key = Key::C;
            for (staff_idx_t i = startStaffIdx; i < endStaffIdx; ++i) {
                Staff* s = score->staff(i);
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
                    interval = Interval(7, 12);
                } else if (direction == TransposeDirection::DOWN) {
                    interval = Interval(-7, -12);
                } else {      //don't do anything for same key and closest direction
                    return true;
                }
            }
        } else {
            interval = Interval::allIntervals[transposeInterval];
            if (direction == TransposeDirection::DOWN) {
                interval.flip();
            }
        }

        if (!rangeSelection) {
            trKeys = false;
        }
        bool fullOctave = (interval.chromatic % PITCH_DELTA_OCTAVE) == 0;
        if (fullOctave && (mode != TransposeMode::TO_KEY)) {
            trKeys = false;
            transposeChordNames = false;
        }
    } else {  // diatonic transposition
        if (direction == TransposeDirection::DOWN) {
            transposeInterval *= -1;
        }
    }

    if (selection.isList()) {
        for (EngravingItem* e : selection.uniqueElements()) {
            if (!e->staff() || e->staff()->staffType(e->tick())->group() == StaffGroup::PERCUSSION) {
                continue;
            }
            if (e->staff()->primaryStaff() && e->staff()->primaryStaff()->staffType(Fraction(0, 1))->group() == StaffGroup::PERCUSSION) {
                continue;
            }
            if (e->isNote()) {
                Note* note = toNote(e);
                if (!transposeNote(note, mode, transposeInterval, trKeys, useDoubleSharpsFlats, interval)) {
                    result = false;
                }
            } else if (e->isHarmony() && transposeChordNames) {
                transposeHarmony(toHarmony(e), score, interval, mode, transposeInterval, trKeys, useDoubleSharpsFlats);
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
                    score->undo(new ChangeKeySig(ks, ke, ks->showCourtesy()));
                }
            }
        }
    }

    //--------------------------
    // process range selection
    //--------------------------

    std::vector<Staff*> sl;
    for (staff_idx_t staffIdx = selection.staffStart(); staffIdx < selection.staffEnd(); ++staffIdx) {
        Staff* s = score->staff(staffIdx);
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
    std::vector<track_idx_t> tracks;
    for (Staff* s : sl) {
        track_idx_t idx = s->idx() * VOICES;
        for (voice_idx_t i = 0; i < VOICES; ++i) {
            if (score->selectionFilter().canSelectVoice(i)) {
                tracks.push_back(idx + i);
            }
        }
    }

    Segment* s1 = selection.startSegment();
    if (!s1) {
        return result;
    }
    // if range start on mmRest, get the actual segment instead
    if (s1->measure()->isMMRest()) {
        s1 = score->tick2segment(s1->tick(), true, s1->segmentType(), false);
    }
    // if range starts with first CR of measure
    // then start looping from very beginning of measure
    // so we include key signature and can transpose that if requested
    if (s1->rtick().isZero()) {
        s1 = s1->measure()->first();
    }
    Segment* s2 = selection.endSegment();
    for (Segment* segment = s1; segment && segment != s2; segment = segment->next1()) {
        if (!segment->enabled()) {
            continue;
        }
        for (track_idx_t track : tracks) {
            if (score->staff(track / VOICES)->staffType(s1->tick())->group() == StaffGroup::PERCUSSION) {
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
                    if (!score->selectionFilter().canSelectNoteIdx(noteIdx, nl.size(), selection.rangeContainsMultiNoteChords())) {
                        continue;
                    }
                    Note* note = nl.at(noteIdx);
                    if (!transposeNote(note, mode, transposeInterval, trKeys, useDoubleSharpsFlats, interval)) {
                        result = false;
                    }
                }
                for (Chord* g : chord->graceNotes()) {
                    for (Note* n : g->notes()) {
                        if (!transposeNote(n, mode, transposeInterval, trKeys, useDoubleSharpsFlats, interval)) {
                            result = false;
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
                    score->undoChangeKeySig(staff, tick, ke);
                }
            }
        }
        if (transposeChordNames && score->selectionFilter().isFiltered(ElementsSelectionFilterTypes::CHORD_SYMBOL)) {
            for (EngravingItem* e : segment->annotations()) {
                if (!e->isHarmony() || (!muse::contains(tracks, e->track()))) {
                    continue;
                }
                transposeHarmony(toHarmony(e), score, interval, mode, transposeInterval, trKeys, useDoubleSharpsFlats);
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
            Segment* seg = score->firstMeasure()->undoGetSegmentR(SegmentType::KeySig, Fraction(0, 1));
            KeySig* ks = toKeySig(seg->element(track));
            if (!ks) {
                ks = Factory::createKeySig(seg);
                ks->setTrack(track);
                Key nKey = transposeKey(Key::C, interval);
                ks->setKey(nKey);
                ks->setParent(seg);
                score->undoAddElement(ks);
            }
        }
    }
    return result;
}

//---------------------------------------------------------
//   transposeKeys
//    key -   -7(Cb) - +7(C#)
//---------------------------------------------------------

void Transpose::transposeKeys(Score* score, staff_idx_t staffStart, staff_idx_t staffEnd, const Fraction& ts, const Fraction& tickEnd,
                              bool flip)
{
    Fraction tickStart(ts);
    if (tickStart < Fraction(0, 1)) {            // -1 and 0 are valid values to indicate start of score
        tickStart = Fraction(0, 1);
    }
    for (staff_idx_t staffIdx = staffStart; staffIdx < staffEnd; ++staffIdx) {
        Staff* st = score->staff(staffIdx);
        if (st->staffType(tickStart)->group() == StaffGroup::PERCUSSION) {
            continue;
        }

        bool createKey = tickStart.isZero();
        for (Segment* s = score->firstSegment(SegmentType::KeySig); s; s = s->next1(SegmentType::KeySig)) {
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
                score->undo(new ChangeKeySig(ks, ke, ks->showCourtesy()));
            }
            if (s->tick().isZero()) {
                createKey = false;
            }
        }
        if (createKey && score->firstMeasure()) {
            Segment* seg = score->firstMeasure()->undoGetSegmentR(SegmentType::KeySig, Fraction(0, 1));
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
            score->undoAddElement(ks);
        }
    }
}

//---------------------------------------------------------
//   transposeSemitone
//---------------------------------------------------------

void Transpose::transposeSemitone(Score* score, int step)
{
    if (step == 0) {
        return;
    }

    TransposeDirection dir = step > 0 ? TransposeDirection::UP : TransposeDirection::DOWN;

    if (!transpose(score, TransposeMode::BY_INTERVAL, dir, Key::C, 1, true, true, false)) {
        LOGD("Score::transposeSemitone: failed");
        // TODO: set error message
    } else {
        score->setSelectionChanged(true);
    }
}

//---------------------------------------------------------
//   transposeDiatonicAlterations
//---------------------------------------------------------
void Transpose::transposeDiatonicAlterations(Score* score, TransposeDirection direction)
{
    // Transpose current selection diatonically (up/down) while keeping degree alterations
    // Note: Score::transpose() absolutely requires valid selection before invocation.
    if (!score->selection().isNone()) {
        transpose(score, TransposeMode::DIATONICALLY, direction, Key::C, 1, true, true, true);
        score->setPlayNote(true);     // For when selection is a single note, also playback that note
        score->setSelectionChanged(true);     // This will update the on-screen keyboard
    }
}

//---------------------------------------------------------
//   transpositionChanged
//---------------------------------------------------------
void Transpose::transpositionChanged(Score* score, Part* part, Interval oldV, Fraction tickStart, Fraction tickEnd)
{
    if (tickStart == Fraction(-1, 1)) {
        tickStart = Fraction(0, 1);
    }

    // transpose keys first
    std::set<Score*> scores;
    for (Staff* ls : part->staff(0)->staffList()) {
        // TODO: special handling for linked staves within a score
        // could be useful for capo
        Score* lsScore = ls->score();
        if (muse::contains(scores, lsScore)) {
            continue;
        }
        scores.insert(lsScore);
        Part* lp = ls->part();
        if (!lsScore->style().styleB(Sid::concertPitch)) {
            transposeKeys(lsScore, lp->startTrack() / VOICES, lp->endTrack() / VOICES, tickStart, tickEnd, true);
        }
    }

    // now transpose notes and chord symbols
    for (Segment* s = score->firstSegment(Segment::CHORD_REST_OR_TIME_TICK_TYPE); s; s = s->next1(Segment::CHORD_REST_OR_TIME_TICK_TYPE)) {
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
                    if (element->track() != track || !element->isHarmony()) {
                        continue;
                    }
                    Harmony* h  = toHarmony(element);
                    for (EngravingObject* scoreElement : h->linkList()) {
                        if (!scoreElement->style().styleB(Sid::concertPitch)) {
                            undoTransposeHarmony(score, toHarmony(scoreElement), diffV, false);
                        }
                    }
                }
            }
        }
    }
}

void Transpose::transpositionChanged(Score* score, Part* part, const Fraction& instrumentTick, Interval oldTransposition)
{
    Fraction tickStart = instrumentTick;
    Fraction tickEnd = { -1, 1 };

    auto mainInstrumentEndIt = part->instruments().upper_bound(tickStart.ticks());
    if (mainInstrumentEndIt != part->instruments().cend()) {
        tickEnd = Fraction::fromTicks(mainInstrumentEndIt->first);
    }

    transpositionChanged(score, part, oldTransposition, tickStart, tickEnd);
}

//---------------------------------------------------------
//   transposeChord
//---------------------------------------------------------

void Transpose::transposeChord(Chord* c, const Fraction& tick)
{
    Interval dstTranspose = c->staff()->transpose(tick);

    if (dstTranspose.isZero()) {
        for (Note* n : c->notes()) {
            n->setTpc2(n->tpc1());
        }
    } else {
        dstTranspose.flip();
        for (Note* n : c->notes()) {
            n->setTpc2(transposeTpc(n->tpc1(), dstTranspose, true));
        }
    }
}

//---------------------------------------------------------
//   keydiff2Interval
//    keysig -   -7(Cb) - +7(C#)
//---------------------------------------------------------

Interval Transpose::keydiff2Interval(Key oKey, Key nKey, TransposeDirection dir)
{
    static int stepTable[STEP_DELTA_OCTAVE] = {
        // C  G  D  A  E  B F
        0, 4, 1, 5, 2, 6, 3,
    };

    int cofSteps = (nKey > oKey) ? int(nKey) - int(oKey) : int(nKey) - int(oKey) + PITCH_DELTA_OCTAVE; // circle of fifth steps
    int diatonic = stepTable[(int(nKey) + 7) % 7] - stepTable[(int(oKey) + 7) % 7];
    if (diatonic < 0) {
        diatonic += STEP_DELTA_OCTAVE;
    }
    diatonic %= STEP_DELTA_OCTAVE;
    int chromatic = (cofSteps * 7) % PITCH_DELTA_OCTAVE;

    if ((dir == TransposeDirection::CLOSEST) && (chromatic > 6)) {
        dir = TransposeDirection::DOWN;
    }

    if (dir == TransposeDirection::DOWN) {
        chromatic = chromatic == 0 ? 0 : chromatic - 12;
        diatonic  = diatonic == 0 ? 0 : diatonic - 7;
    }
    return Interval(diatonic, chromatic);
}

bool Transpose::transposeNote(Note* note, TransposeMode mode, int transposeInterval, bool trKeys, bool useDoubleSharpsFlats,
                              Interval interval)
{
    if (mode == TransposeMode::DIATONICALLY) {
        return note->transposeDiatonic(transposeInterval, trKeys, useDoubleSharpsFlats);
    }

    return note->transpose(interval, useDoubleSharpsFlats);
}

void Transpose::transposeHarmony(Harmony* harmony, Score* score, Interval interval, TransposeMode mode, int transposeInterval, bool trKeys,
                                 bool useDoubleSharpsFlats)
{
    // TODO also source interval should reflect modified key (f.ex. by prefer flat)
    // now we calculate interval from first pitched staff
    // but even if it were right staff, this may differ each tick (actual key signature)
    // we can  add something like "concert pitch rootTpc" to harmony definition, or ...
    Interval kv = harmony->staff()->transpose(harmony->tick());
    Interval iv = harmony->part()->instrument(harmony->tick())->transpose();
    Interval hInterval((interval.diatonic - kv.diatonic + iv.diatonic), (interval.chromatic - kv.chromatic + iv.chromatic));

    // undoTransposeHarmony does not do links
    // because it is also used to handle transposing instruments
    // and score / parts could be in different concert pitch states
    for (EngravingObject* se : harmony->linkList()) {
        Harmony* h = toHarmony(se);
        if (mode == TransposeMode::DIATONICALLY) {
            undoTransposeHarmonyDiatonic(score, h, transposeInterval, useDoubleSharpsFlats, trKeys);
        } else {
            undoTransposeHarmony(score, h, hInterval, useDoubleSharpsFlats);
        }
    }
}

//---------------------------------------------------------
//   transposeTpcDiatonicByKey
//
// returns the tpc diatonically transposed by steps, using degrees of given key
// option to keep any alteration tpc had with respect to unaltered corresponding degree of key
// option to enharmonically reduce tpc using double alterations
//---------------------------------------------------------

int Transpose::transposeTpcDiatonicByKey(int tpc, int steps, Key key, bool keepAlteredDegrees, bool useDoubleSharpsFlats)
{
    if (!tpcIsValid(tpc)) {
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
    return clampEnharmonic(newTpc, useDoubleSharpsFlats);
}

//---------------------------------------------------------
//   transposeKey
//---------------------------------------------------------

Key Transpose::transposeKey(Key key, const Interval& interval, PreferSharpFlat prefer)
{
    Key newKey = Key(transposeTpc(int(key), interval, false));

    // ignore prefer for octave transposing instruments
    if (interval.chromatic % PITCH_DELTA_OCTAVE == 0 && interval.diatonic % STEP_DELTA_OCTAVE == 0) {
        prefer = PreferSharpFlat::NONE;
    }

    return clampKey(newKey, prefer);
}

//---------------------------------------------------------
//   transposeTpc
//
// transposes a pitch spelling by a given interval.
// option to enharmonically reduce tpc using double alterations
//---------------------------------------------------------

int Transpose::transposeTpc(int tpc, Interval interval, bool useDoubleSharpsFlats)
{
    int deltaTpc = (interval.chromatic * TPC_DELTA_SEMITONE) - (interval.diatonic * TPC_DELTA_ENHARMONIC);
    if (!tpcIsValid(tpc) || deltaTpc == 0) { // perfect unison & perfect octave
        return tpc;
    }
    return clampEnharmonic(tpc + deltaTpc, useDoubleSharpsFlats);
}

void Transpose::undoTransposeHarmony(Score* score, Harmony* harmony, Interval interval, bool doubleSharpFlat)
{
    score->undo(new TransposeHarmony(harmony, interval, doubleSharpFlat));
}

void Transpose::undoTransposeHarmonyDiatonic(Score* score, Harmony* harmony, int interval, bool doubleSharpFlat, bool transposeKeys)
{
    score->undo(new TransposeHarmonyDiatonic(harmony, interval, doubleSharpFlat, transposeKeys));
}

//---------------------------------------------------------
//   TransposeHarmony
//---------------------------------------------------------

TransposeHarmony::TransposeHarmony(Harmony* h, Interval interval, bool doubleSharpsFlats)
{
    m_harmony = h;
    m_interval = interval;
    m_useDoubleSharpsFlats = doubleSharpsFlats;
}

void TransposeHarmony::flip(EditData*)
{
    m_harmony->realizedHarmony().setDirty(true);   // harmony should be re-realized after transposition

    for (HarmonyInfo* info : m_harmony->chords()) {
        info->setRootTpc(Transpose::transposeTpc(info->rootTpc(), m_interval, m_useDoubleSharpsFlats));
        info->setBassTpc(Transpose::transposeTpc(info->bassTpc(), m_interval, m_useDoubleSharpsFlats));
    }

    m_harmony->setXmlText(m_harmony->harmonyName());
    m_harmony->triggerLayout();
    m_interval.flip();
}

//---------------------------------------------------------
//   TransposeHarmonyDiatonic
//---------------------------------------------------------

TransposeHarmonyDiatonic::TransposeHarmonyDiatonic(Harmony* h, int interval, bool useDoubleSharpsFlats, bool transposeKeys)
{
    m_harmony = h;
    m_interval = interval;
    m_useDoubleSharpsFlats = useDoubleSharpsFlats;
    m_transposeKeys = transposeKeys;
}

void TransposeHarmonyDiatonic::flip(EditData*)
{
    m_harmony->realizedHarmony().setDirty(true);   // harmony should be re-realized after transposition

    Fraction tick = Fraction(0, 1);
    Segment* seg = toSegment(m_harmony->findAncestor(ElementType::SEGMENT));
    if (seg) {
        tick = seg->tick();
    }
    Key key = !m_harmony->staff() ? Key::C : m_harmony->staff()->key(tick);

    for (HarmonyInfo* info : m_harmony->chords()) {
        info->setRootTpc(Transpose::transposeTpcDiatonicByKey(info->rootTpc(), m_interval, key, m_transposeKeys, m_useDoubleSharpsFlats));
        info->setBassTpc(Transpose::transposeTpcDiatonicByKey(info->bassTpc(), m_interval, key, m_transposeKeys, m_useDoubleSharpsFlats));
    }

    m_harmony->setXmlText(m_harmony->harmonyName());
    m_harmony->triggerLayout();

    m_interval *= -1;
}
