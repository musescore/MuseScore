/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2026 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "exchangevoices.h"

#include "../dom/excerpt.h"
#include "../dom/measure.h"
#include "../dom/rest.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/staff.h"
#include "../types/types.h"

#include "clonevoice.h"
#include "undo.h"

using namespace mu::engraving;

namespace {
class ExchangeVoicesInMeasure : public UndoCommand
{
    OBJECT_ALLOCATOR(engraving, ExchangeVoicesInMeasure)

public:
    ExchangeVoicesInMeasure(Measure* measure, track_idx_t srcTrack, track_idx_t dstTrack, staff_idx_t staff)
        : m_measure(measure), m_srcTrack(srcTrack), m_dstTrack(dstTrack), m_staff(staff) {}

    void undo(EditData*) override
    {
        doExchange(m_dstTrack, m_srcTrack);
    }

    void redo(EditData*) override
    {
        doExchange(m_srcTrack, m_dstTrack);
    }

    UNDO_TYPE(CommandType::ExchangeVoice)
    UNDO_NAME("ExchangeVoice")
    UNDO_CHANGED_OBJECTS({ m_measure })

private:
    void doExchange(track_idx_t srcTrack, track_idx_t dstTrack);

    Measure* m_measure = nullptr;
    track_idx_t m_srcTrack = muse::nidx;
    track_idx_t m_dstTrack = muse::nidx;
    staff_idx_t m_staff = muse::nidx;
};

void ExchangeVoicesInMeasure::doExchange(track_idx_t srcTrack, track_idx_t dstTrack)
{
    for (Segment* s = m_measure->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
        s->swapElements(srcTrack, dstTrack);
    }

    Fraction start = m_measure->tick();
    Fraction end = start + m_measure->ticks();
    auto spanners = m_measure->score()->spannerMap().findOverlapping(start.ticks(), end.ticks() - 1);
    for (auto i = spanners.begin(); i < spanners.end(); i++) {
        Spanner* sp = i->value;
        Fraction spStart = sp->tick();
        Fraction spEnd = spStart + sp->ticks();
        LOGD("Start %d End %d Diff %d \n Measure Start %d End %d", spStart.ticks(), spEnd.ticks(), (spEnd - spStart).ticks(),
             start.ticks(), end.ticks());
        if (sp->isSlur() && (spStart >= start || spEnd < end)) {
            if (sp->track() == srcTrack && spStart >= start) {
                sp->setTrack(dstTrack);
            } else if (sp->track() == dstTrack && spStart >= start) {
                sp->setTrack(srcTrack);
            }
            if (sp->track2() == srcTrack && spEnd < end) {
                sp->setTrack2(dstTrack);
            } else if (sp->track2() == dstTrack && spEnd < end) {
                sp->setTrack2(srcTrack);
            }
        }
    }
    m_measure->checkMultiVoices(m_staff); // probably true, but check for invisible notes & rests
}
}

void ExchangeVoices::exchangeVoicesInSelection(Score* score, voice_idx_t srcVoice, voice_idx_t dstVoice)
{
    if (!score->selection().isRange()) {
        MScore::setError(MsError::NO_STAFF_SELECTED);
        return;
    }
    Fraction t1 = score->selection().tickStart();
    Fraction t2 = score->selection().tickEnd();

    Measure* m1 = score->tick2measure(t1);
    Measure* m2 = score->tick2measure(t2);

    if (score->excerpt()) {
        return;
    }

    if (t2 > m2->tick()) {
        m2 = m2->nextMeasure();
    }

    for (;;) {
        exchangeVoices(score, m1, srcVoice, dstVoice, score->selection().staffStart(), score->selection().staffEnd());
        m1 = m1->nextMeasure();
        if ((m1 == 0) || (m2 && (m1->tick() == m2->tick()))) {
            break;
        }
    }
}

void ExchangeVoices::exchangeVoices(Score* score, Measure* measure, voice_idx_t srcVoice, voice_idx_t dstVoice, staff_idx_t srcStaff,
                                    staff_idx_t dstStaff)
{
    Fraction tick = measure->tick();

    for (staff_idx_t staffIdx = srcStaff; staffIdx < dstStaff; ++staffIdx) {
        track_idx_t srcStaffTrack = staffIdx * VOICES;
        track_idx_t srcTrack = srcStaffTrack + srcVoice;
        track_idx_t dstTrack = srcStaffTrack + dstVoice;
        int trackDiff = static_cast<int>(dstVoice - srcVoice);

        //handle score and complete measures first
        score->undo(new ExchangeVoicesInMeasure(measure, srcTrack, dstTrack, staffIdx));

        for (Staff* st : score->staff(staffIdx)->staffList()) {
            track_idx_t staffTrack = st->idx() * VOICES;
            Measure* measure2 = st->score()->tick2measure(tick);
            Excerpt* ex = st->score()->excerpt();

            if (ex) {
                const TracksMap& tracks = ex->tracksMapping();
                std::vector<track_idx_t> srcTrackList = muse::values(tracks, srcTrack);
                std::vector<track_idx_t> dstTrackList = muse::values(tracks, dstTrack);

                for (track_idx_t srcTrack2 : srcTrackList) {
                    // don't care about other linked staves
                    if (!(staffTrack <= srcTrack2) || !(srcTrack2 < staffTrack + VOICES)) {
                        continue;
                    }

                    track_idx_t tempTrack = srcTrack;
                    std::vector<track_idx_t> testTracks = muse::values(tracks, tempTrack + trackDiff);
                    bool hasVoice = false;
                    for (track_idx_t testTrack : testTracks) {
                        if (staffTrack <= testTrack && testTrack < staffTrack + VOICES && muse::contains(dstTrackList, testTrack)) {
                            hasVoice = true;
                            // voice is simply exchangeable now (deal directly)
                            score->undo(new ExchangeVoicesInMeasure(measure2, srcTrack2, testTrack, staffTrack / 4));
                        }
                    }

                    // only source voice is in this staff
                    if (!hasVoice) {
                        CloneVoice::cloneVoice(measure->first(), measure2->endTick(), measure2->first(), tempTrack, srcTrack2, false, true);
                        muse::remove(srcTrackList, srcTrack2);
                    }
                }

                for (track_idx_t dstTrack2 : dstTrackList) {
                    // don't care about other linked staves
                    if (!(staffTrack <= dstTrack2) || !(dstTrack2 < staffTrack + VOICES)) {
                        continue;
                    }

                    track_idx_t tempTrack = dstTrack;
                    std::vector<track_idx_t> testTracks = muse::values(tracks, tempTrack - trackDiff);
                    bool hasVoice = false;
                    for (track_idx_t testTrack : testTracks) {
                        if (staffTrack <= testTrack && testTrack < staffTrack + VOICES && muse::contains(srcTrackList, testTrack)) {
                            hasVoice = true;
                        }
                    }

                    // only destination voice is in this staff
                    if (!hasVoice) {
                        CloneVoice::cloneVoice(measure->first(), measure2->endTick(), measure2->first(), tempTrack, dstTrack2, false, true);
                        muse::remove(dstTrackList, dstTrack2);
                    }
                }
            } else if (srcStaffTrack != staffTrack) {
                // linked staff in same score (all voices present can be assumed)
                score->undo(new ExchangeVoicesInMeasure(measure2, staffTrack + srcVoice, staffTrack + dstVoice, st->idx()));
            }
        }
    }

    // make sure voice 0 is complete

    if (srcVoice == 0 || dstVoice == 0) {
        for (staff_idx_t staffIdx = srcStaff; staffIdx < dstStaff; ++staffIdx) {
            // check for complete timeline of voice 0
            Fraction ctick  = measure->tick();
            track_idx_t track = staffIdx * VOICES;
            for (Segment* s = measure->first(SegmentType::ChordRest); s; s = s->next(SegmentType::ChordRest)) {
                ChordRest* cr = toChordRest(s->element(track));
                if (!cr) {
                    continue;
                }
                if (cr->isRest()) {
                    Rest* r = toRest(cr);
                    if (r->isGap()) {
                        r->undoChangeProperty(Pid::GAP, false);
                    }
                }
                if (ctick < s->tick()) {
                    score->setRest(ctick, track, s->tick() - ctick, false, 0); // fill gap
                }
                ctick = cr->endTick();
            }
            Fraction etick = measure->endTick();
            if (ctick < etick) {
                score->setRest(ctick, track, etick - ctick, false, 0); // fill gap
            }
        }
    }
}
