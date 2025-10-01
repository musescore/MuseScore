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

#include "editmeasures.h"

#include "../dom/chord.h"
#include "../dom/chordrest.h"
#include "../dom/clef.h"
#include "../dom/engravingitem.h"
#include "../dom/instrchange.h"
#include "../dom/key.h"
#include "../dom/keysig.h"
#include "../dom/measure.h"
#include "../dom/note.h"
#include "../dom/page.h"
#include "../dom/part.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/spanner.h"
#include "../dom/staff.h"
#include "../dom/stafftypechange.h"
#include "../dom/system.h"
#include "../dom/tie.h"
#include "../dom/utils.h"

#include "log.h"

using namespace muse;
using namespace mu::engraving;

//---------------------------------------------------------
//   getCourtesyClefs
//    remember clefs at the end of previous measure
//---------------------------------------------------------

std::vector<Clef*> InsertRemoveMeasures::getCourtesyClefs(Measure* m)
{
    Score* score = m->score();
    std::vector<Clef*> startClefs;
    if (m->prev() && m->prev()->isMeasure()) {
        Measure* prevMeasure = toMeasure(m->prev());
        const Segment* clefSeg = prevMeasure->findSegmentR(SegmentType::Clef | SegmentType::HeaderClef, prevMeasure->ticks());
        if (clefSeg) {
            for (size_t st = 0; st < score->nstaves(); ++st) {
                EngravingItem* clef = clefSeg->element(staff2track(static_cast<int>(st)));
                if (clef && clef->isClef()) {
                    startClefs.push_back(toClef(clef));
                }
            }
        }
    }
    return startClefs;
}

//---------------------------------------------------------
//   insertMeasures
//---------------------------------------------------------

void InsertRemoveMeasures::insertMeasures()
{
    Score* score = fm->score();

    std::vector<Clef*> clefs;
    std::vector<Clef*> prevMeasureClefs;
    std::vector<KeySig*> keys;
    Segment* fs = nullptr;
    Segment* ls = nullptr;
    if (fm->isMeasure()) {
        score->setPlaylistDirty();
        fs = toMeasure(fm)->first();
        ls = toMeasure(lm)->last();
        for (Segment* s = fs; s && s != ls; s = s->next1()) {
            if (!s->enabled() || !(s->segmentType() & (SegmentType::Clef | SegmentType::HeaderClef | SegmentType::KeySig))) {
                continue;
            }
            for (track_idx_t track = 0; track < score->ntracks(); track += VOICES) {
                EngravingItem* e = s->element(track);
                if (!e || e->generated()) {
                    continue;
                }
                if (e->isClef()) {
                    clefs.push_back(toClef(e));
                } else if (e->isKeySig()) {
                    keys.push_back(toKeySig(e));
                }
            }
        }
        prevMeasureClefs = getCourtesyClefs(toMeasure(fm));
    }
    score->measures()->insert(fm, lm);

    if (fm->isMeasure()) {
        score->setUpTempoMap();
        score->insertTime(fm->tick(), lm->endTick() - fm->tick());

        // move ownership of Instrument back to part
        for (Segment* s = fs; s && s != ls; s = s->next1()) {
            for (EngravingItem* e : s->annotations()) {
                if (e->isInstrumentChange()) {
                    e->part()->setInstrument(toInstrumentChange(e)->instrument(), s->tick());
                }
            }
        }
        for (Clef* clef : prevMeasureClefs) {
            clef->staff()->setClef(clef);
        }
        for (Clef* clef : clefs) {
            clef->staff()->setClef(clef);
        }
        for (KeySig* key : keys) {
            key->staff()->setKey(key->segment()->tick(), key->keySigEvent());
        }
    }

    score->setLayoutAll();

    // move subsequent StaffTypeChanges
    Fraction tickStart = fm->tick();
    Fraction tickEnd = lm->endTick();
    if (moveStc && tickEnd > tickStart) {
        for (Staff* staff : score->staves()) {
            // loop backwards until the insert point
            auto stRange = staff->staffTypeRange(score->lastMeasure()->tick());
            int moveTick = stRange.first;

            while (moveTick >= tickStart.ticks() && moveTick > 0) {
                Fraction tick = Fraction::fromTicks(moveTick);
                Fraction tickNew = tick + tickEnd - tickStart;

                // measures were inserted already so icon is at differnt place, as staffTypeChange itslef
                Measure* measure = score->tick2measure(tickNew);
                bool stIcon = false;

                staff->moveStaffType(tick, tickNew);

                for (EngravingItem* el : measure->el()) {
                    if (el && el->isStaffTypeChange() && el->track() == staff->idx() * VOICES) {
                        StaffTypeChange* stc = toStaffTypeChange(el);
                        stc->setStaffType(staff->staffType(tickNew), false);
                        stIcon = true;
                        break;
                    }
                }

                if (!stIcon) {
                    LOGD() << "StaffTypeChange icon is missing in measure " << measure->no();
                }

                stRange = staff->staffTypeRange(tick);
                moveTick = stRange.first;
            }
        }
    }

    //
    // connect ties
    //

    if (!fm->isMeasure() || !fm->prevMeasure()) {
        return;
    }
    Measure* m = fm->prevMeasure();
    for (Segment* seg = m->first(); seg; seg = seg->next()) {
        for (track_idx_t track = 0; track < score->ntracks(); ++track) {
            EngravingItem* e = seg->element(track);
            if (!e || !e->isChord()) {
                continue;
            }
            Chord* chord = toChord(e);
            for (Note* n : chord->notes()) {
                Tie* tie = n->tieForNonPartial();
                if (!tie) {
                    continue;
                }
                if (!tie->endNote() || tie->endNote()->chord()->segment()->measure() != m) {
                    Note* nn = searchTieNote(n);
                    if (nn) {
                        tie->setEndNote(nn);
                        nn->setTieBack(tie);
                    }
                }
            }
        }
    }
}

//---------------------------------------------------------
//   removeMeasures
//---------------------------------------------------------

void InsertRemoveMeasures::removeMeasures()
{
    Score* score = fm->score();

    Fraction tick1 = fm->tick();
    Fraction tick2 = lm->endTick();

    if (fm->isMeasure() && lm->isMeasure()) {
        // remove beams from chordrests in affected area, they will be rebuilt later but we need
        // to avoid situations where notes from deleted measures remain in beams
        // when undoing, we need to check the previous measure as well as there could be notes in there
        // that need to have their beams recalculated (esp. when adding time signature)
        MeasureBase* prev = fm->prev();
        Segment* first = toMeasure(prev && prev->isMeasure() ? prev : fm)->first();
        for (Segment* s = first; s && s != toMeasure(lm)->last(); s = s->next1()) {
            if (!s) {
                break;
            }
            if (!s->isChordRestType()) {
                continue;
            }

            for (track_idx_t track = 0; track < score->ntracks(); ++track) {
                EngravingItem* e = s->element(track);
                if (e && e->isChordRest()) {
                    toChordRest(e)->removeDeleteBeam(false);
                }
            }
        }
    }

    std::vector<System*> systemList;
    for (MeasureBase* mb = lm;; mb = mb->prev()) {
        System* system = mb->system();
        if (system) {
            if (!muse::contains(systemList, system)) {
                systemList.push_back(system);
            }
            system->removeMeasure(mb);
        }
        if (mb == fm) {
            break;
        }
    }

    // move subsequent StaffTypeChanges
    if (moveStc) {
        for (Staff* staff : score->staves()) {
            Fraction tickStart = tick1;
            Fraction tickEnd = tick2;

            // loop trhu, until the last one
            auto stRange = staff->staffTypeRange(tickEnd);
            int moveTick = stRange.first == tickEnd.ticks() ? stRange.first : stRange.second;

            while (moveTick != -1) {
                Fraction tick = Fraction::fromTicks(moveTick);
                Fraction newTick = tick + tickStart - tickEnd;

                Measure* measure = score->tick2measure(tick);
                bool stIcon = false;

                staff->moveStaffType(tick, newTick);

                for (EngravingItem* el : measure->el()) {
                    if (el && el->isStaffTypeChange() && el->track() == staff->idx() * VOICES) {
                        StaffTypeChange* stc = toStaffTypeChange(el);
                        stc->setStaffType(staff->staffType(newTick), false);
                        stIcon = true;
                        break;
                    }
                }

                if (!stIcon) {
                    LOGD() << "StaffTypeChange icon is missing in measure " << measure->no();
                }

                stRange = staff->staffTypeRange(tick);
                moveTick = stRange.second;
            }
        }
    }

    score->measures()->remove(fm, lm);

    if (fm->isMeasure()) {
        score->setUpTempoMap();
        score->setPlaylistDirty();

        // check if there is a clef at the end of last measure
        // remove clef from staff cleflist

        if (lm->isMeasure()) {
            Measure* m = toMeasure(lm);
            Segment* s = m->findSegment(SegmentType::Clef, tick2);
            if (s) {
                for (EngravingItem* e : s->elist()) {
                    Clef* clef = toClef(e);
                    if (clef) {
                        score->staff(clef->staffIdx())->removeClef(clef);
                    }
                }
            }
        }

        // remember clefs at the end of previous measure
        const auto clefs = getCourtesyClefs(toMeasure(fm));

        score->insertTime(tick1, -(tick2 - tick1));

        // Restore clefs that were backed up. Events for them could be lost
        // as a result of the recent insertTime() call.
        for (Clef* clef : clefs) {
            clef->staff()->setClef(clef);
        }

        std::set<Spanner*> spannersCopy = score->unmanagedSpanners();
        for (Spanner* sp : spannersCopy) {
            if ((sp->tick() >= tick1 && sp->tick() < tick2) || (sp->tick2() >= tick1 && sp->tick2() < tick2)) {
                sp->removeUnmanaged();
            }
        }
    }

    // remove empty systems

    for (System* s : systemList) {
        if (s->measures().empty()) {
            Page* page = s->page();
            if (page) {
                // erase system from page
                muse::remove(page->systems(), s);
                // erase system from score
                muse::remove(score->systems(), s);
                // finally delete system
                score->deleteLater(s);
            }
        }
    }

    score->setLayoutAll();
}

//---------------------------------------------------------
//   ChangeMeasureLen
//---------------------------------------------------------

ChangeMeasureLen::ChangeMeasureLen(Measure* m, Fraction l)
{
    measure     = m;
    len         = l;
}

void ChangeMeasureLen::flip(EditData*)
{
    Fraction oLen = measure->ticks();

    //
    // move EndBarLine and TimeSigAnnounce
    // to end of measure:
    //

    for (Segment* s = measure->first(); s; s = s->next()) {
        if (!s->isEndBarLineType() && !s->isTimeSigAnnounceType()) {
            continue;
        }
        s->setRtick(len);
        measure->remove(s);
    }
    measure->setTicks(len);
    measure->score()->setUpTempoMap();
    len = oLen;
}

//---------------------------------------------------------
//   ChangeMMRest
//---------------------------------------------------------

void ChangeMMRest::flip(EditData*)
{
    Measure* mmr = m->mmRest();
    m->setMMRest(mmrest);
    mmrest = mmr;
}

//---------------------------------------------------------
//   ChangeMeasureRepeatCount
//---------------------------------------------------------

void ChangeMeasureRepeatCount::flip(EditData*)
{
    int oldCount = m->measureRepeatCount(staffIdx);
    m->setMeasureRepeatCount(count, staffIdx);
    count = oldCount;
}
