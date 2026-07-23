/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited and others
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

#include "editspanner.h"

#include "../dom/chord.h"
#include "../dom/engravingitem.h"
#include "../dom/measure.h"
#include "../dom/note.h"
#include "../dom/score.h"
#include "../dom/segment.h"
#include "../dom/spanner.h"
#include "../dom/textlinebase.h"
#include "../dom/tie.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   EditSpanner
//---------------------------------------------------------

void EditSpanner::addSpanner(Transaction&, Score* score, Spanner* spanner, const PointF& pos, bool systemStavesOnly)
{
    staff_idx_t staffIdx = spanner->staffIdx();
    Segment* segment;
    MeasureBase* mb = score->pos2measure(pos, &staffIdx, 0, &segment, 0);
    if (systemStavesOnly) {
        staffIdx = 0;
    }
    // ignore if we do not have a measure
    if (mb == 0 || !mb->isMeasure()) {
        LOGD("addSpanner: cannot put object here");
        delete spanner;
        return;
    }

    // all spanners live in voice 0 (except slurs/ties)
    track_idx_t track = staffIdx == muse::nidx ? muse::nidx : staffIdx * VOICES;

    spanner->setTrack(track);
    spanner->setTrack2(track);

    if (spanner->anchor() == Spanner::Anchor::SEGMENT) {
        spanner->setTick(segment->tick());
        Fraction tick2 = std::min(segment->measure()->endTick(), score->lastMeasure()->endTick());
        spanner->setTick2(tick2);
    } else {      // Anchor::MEASURE, Anchor::CHORD, Anchor::NOTE
        Measure* m = toMeasure(mb);
        spanner->setTick(m->tick());
        spanner->setTick2(m->endTick());
    }
    spanner->eraseSpannerSegments();

    bool ctrlModifier = isSystemTextLine(spanner) && !systemStavesOnly;
    score->undoAddElement(spanner, true /*addToLinkedStaves*/, ctrlModifier);
}

void EditSpanner::addSpanner(Transaction&, Score* score, Spanner* spanner, staff_idx_t staffIdx, Segment* startSegment,
                             Segment* endSegment, bool ctrlModifier)
{
    track_idx_t track = staffIdx * VOICES;
    spanner->setTrack(track);
    spanner->setTrack2(track);
    for (auto ss : spanner->spannerSegments()) {
        ss->setTrack(track);
    }

    bool isMeasureAnchor = spanner->anchor() == Spanner::Anchor::MEASURE;
    Fraction tick1 = isMeasureAnchor ? startSegment->measure()->tick() : startSegment->tick();
    spanner->setTick(tick1);

    Fraction tick2;
    if (!endSegment) {
        tick2 = score->lastSegment()->tick();
    } else if (endSegment == startSegment) {
        tick2 = startSegment->measure()->last()->tick();
    } else {
        tick2 = endSegment->tick();
    }
    if (isMeasureAnchor) {
        Measure* endMeasure = score->tick2measureMM(tick2);
        if (endMeasure->tick() != tick2) {
            tick2 = endMeasure->endTick();
        }
    }

    spanner->setTick2(tick2);
    score->undoAddElement(spanner, true, ctrlModifier);
}

//---------------------------------------------------------
//   ChangeSpannerElements
//---------------------------------------------------------

void ChangeSpannerElements::flip()
{
    EngravingItem* oldStartElement   = spanner->startElement();
    EngravingItem* oldEndElement     = spanner->endElement();
    bool isPartialSpanner = spanner->isPartialTie() || spanner->isLaissezVib();
    if (spanner->anchor() == Spanner::Anchor::NOTE) {
        // be sure new spanner elements are of the right type
        if (!isPartialSpanner && (!startElement || !startElement->isNote() || !endElement || !endElement->isNote())) {
            return;
        }
        Note* oldStartNote = toNote(oldStartElement);
        Note* oldEndNote = toNote(oldEndElement);
        Note* newStartNote = toNote(startElement);
        Note* newEndNote = toNote(endElement);
        // update spanner's start and end notes
        if ((newStartNote && newEndNote) || (isPartialSpanner && (newStartNote || newEndNote))) {
            spanner->setNoteSpan(newStartNote, newEndNote);
            if (spanner->isTie()) {
                Tie* tie = toTie(spanner);
                if (oldStartNote && newStartNote) {
                    oldStartNote->setTieFor(nullptr);
                    newStartNote->setTieFor(tie);
                }
                if (oldEndNote && newEndNote) {
                    oldEndNote->setTieBack(nullptr);
                    newEndNote->setTieBack(tie);
                }
            } else {
                oldStartNote->removeSpannerFor(spanner);
                oldEndNote->removeSpannerBack(spanner);
                newStartNote->addSpannerFor(spanner);
                newEndNote->addSpannerBack(spanner);
                if (spanner->isGlissando()) {
                    oldEndNote->chord()->updateEndsNoteAnchoredLine();
                }
            }
        }
    } else {
        spanner->setStartElement(startElement);
        spanner->setEndElement(endElement);
    }
    startElement = oldStartElement;
    endElement   = oldEndElement;
    spanner->triggerLayout();
}

//---------------------------------------------------------
//   ChangeStartEndSpanner
//---------------------------------------------------------

void ChangeStartEndSpanner::flip()
{
    EngravingItem* s = spanner->startElement();
    EngravingItem* e = spanner->endElement();
    spanner->setStartElement(start);
    spanner->setEndElement(end);
    start = s;
    end   = e;
}
