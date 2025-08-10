/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2025 MuseScore Limited
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

#include "tiejumppointlist.h"

#include "barline.h"
#include "chord.h"
#include "factory.h"
#include "marker.h"
#include "measure.h"
#include "note.h"
#include "partialtie.h"
#include "score.h"
#include "segment.h"
#include "tie.h"
#include "undo.h"
#include "volta.h"

using namespace mu::engraving;

//---------------------------------------------------------
//   TieJumpPoint
//---------------------------------------------------------

TieJumpPoint::TieJumpPoint(Note* note, bool active, int idx, bool followingNote)
    : m_note(note), m_active(active), m_followingNote(followingNote)
{
    m_id = u"jumpPoint" + String::fromStdString(std::to_string(idx));
    if (active && endTie()) {
        endTie()->setJumpPoint(this);
    }
}

Tie* TieJumpPoint::endTie() const
{
    return m_note ? m_note->tieBack() : nullptr;
}

void TieJumpPoint::undoSetActive(bool v)
{
    Score* score = m_note ? m_note->score() : nullptr;
    if (!score || m_active == v) {
        return;
    }
    score->undo(new ChangeTieJumpPointActive(m_jumpPointList, m_id, v));
}

const String TieJumpPoint::menuTitle() const
{
    const Measure* measure = m_note->findMeasure();
    const int measureNo = measure ? measure->no() + 1 : 0;

    //: %1 represents the preceding jump item eg. coda. %2 represents the measure number
    return muse::mtrc("engraving", "Tie to %1 (m. %2)").arg(precedingJumpItemName(), String::number(measureNo));
}

String TieJumpPoint::precedingJumpItemName() const
{
    const Chord* startChord = m_note->chord();
    const Segment* seg = startChord->segment();
    const Measure* measure = seg->measure();

    if (seg->score()->firstSegment(SegmentType::ChordRest) == seg) {
        //: Used at %1 in the string "Tie to %1 (m. %2)"
        return muse::mtrc("engraving", "start of score", "partial tie menu");
    }

    // Markers
    for (const EngravingItem* e : measure->el()) {
        if (!e->isMarker()) {
            continue;
        }

        const Marker* marker = toMarker(e);
        if (marker->isRightMarker()) {
            continue;
        }

        if (marker->markerType() == MarkerType::CODA || marker->markerType() == MarkerType::VARCODA) {
            //: Used at %1 in the string "Tie to %1 (m. %2)"
            return muse::mtrc("engraving", "coda", "partial tie menu");
        } else {
            //: Used at %1 in the string "Tie to %1 (m. %2)"
            return muse::mtrc("engraving", "segno", "partial tie menu");
        }
    }

    // Voltas
    auto spanners = m_note->score()->spannerMap().findOverlapping(measure->tick().ticks(), measure->tick().ticks());
    for (auto& spanner : spanners) {
        if (!spanner.value->isVolta() || Fraction::fromTicks(spanner.start) != startChord->tick()) {
            continue;
        }

        Volta* volta = toVolta(spanner.value);

        //: Used at %1 in the string "Tie to %1 (m. %2)". %1 in this string represents the volta's text set by the user
        return muse::mtrc("engraving", "“%1” volta", "partial tie menu").arg(volta->beginText());
    }

    // Repeat barlines
    if (measure->repeatStart()) {
        //: Used at %1 in the string "Tie to %1 (m. %2)"
        return muse::mtrc("engraving", "start repeat", "partial tie menu");
    }

    for (Segment* prevSeg = seg->prev(SegmentType::BarLineType); prevSeg && prevSeg->tick() == seg->tick();
         prevSeg = prevSeg->prev(SegmentType::BarLineType)) {
        EngravingItem* el = prevSeg->element(startChord->track());
        if (!el || !el->isBarLine()) {
            continue;
        }

        BarLine* bl = toBarLine(el);
        if (bl->barLineType() & (BarLineType::START_REPEAT | BarLineType::END_START_REPEAT)) {
            //: Used at %1 in the string "Tie to %1 (m. %2)"
            return muse::mtrc("engraving", "start repeat", "partial tie menu");
        }
    }

    if (m_followingNote) {
        //: Used at %1 in the string "Tie to %1 (m. %2)"
        return muse::mtrc("engraving", "next note", "partial tie menu");
    }

    //: Used at %1 in the string "Tie to %1 (m. %2)"
    return muse::mtrc("engraving", "invalid", "partial tie menu");
}

//---------------------------------------------------------
//   TieJumpPointList
//---------------------------------------------------------

TieJumpPointList::~TieJumpPointList()
{
    muse::DeleteAll(m_jumpPoints);
    m_jumpPoints.clear();
}

void TieJumpPointList::add(TieJumpPoint* item)
{
    item->setJumpPointList(this);
    m_jumpPoints.push_back(item);
}

void TieJumpPointList::clear()
{
    for (const TieJumpPoint* jumpPoint : m_jumpPoints) {
        Tie* endTie = jumpPoint->endTie();
        if (!endTie) {
            continue;
        }
        endTie->setJumpPoint(nullptr);
    }
    muse::DeleteAll(m_jumpPoints);
    m_jumpPoints.clear();
}

Tie* TieJumpPointList::startTie() const
{
    return m_note ? m_note->tieFor() : nullptr;
}

TieJumpPoint* TieJumpPointList::findJumpPoint(const String& id)
{
    for (TieJumpPoint* jumpPoint : m_jumpPoints) {
        if (jumpPoint->id() != id) {
            continue;
        }

        return jumpPoint;
    }
    return nullptr;
}

void TieJumpPointList::toggleJumpPoint(const String& id)
{
    TieJumpPoint* end = findJumpPoint(id);

    if (!end) {
        LOGE() << "No partial tie end point found with id: " << id;
        return;
    }

    Score* score = end->note() ? end->note()->score() : nullptr;
    if (!score) {
        return;
    }

    score->startCmd(TranslatableString("engraving", "Toggle partial tie"));
    const bool checked = end->active();
    if (checked) {
        undoRemoveTieFromScore(end);
    } else {
        undoAddTieToScore(end);
    }
    score->endCmd();
}

void TieJumpPointList::undoAddTieToScore(TieJumpPoint* jumpPoint)
{
    Note* note = jumpPoint->note();
    Score* score = note ? note->score() : nullptr;
    Tie* tie = startTie();
    if (!tie || !score) {
        return;
    }

    if (jumpPoint->followingNote()) {
        // Remove partial tie and add full tie
        if (!tie->isPartialTie() || !toPartialTie(tie)->isOutgoing()) {
            return;
        }
        jumpPoint->undoSetActive(true);
        Tie::changeTieType(tie, note);
        return;
    }

    jumpPoint->undoSetActive(true);

    // Check if there is already a tie.  If so, add partial tie info to it
    Tie* tieBack = note->tieBack();
    if (tieBack && !tieBack->isPartialTie()) {
        tieBack->setJumpPoint(jumpPoint);
        return;
    }
    // Otherwise create incoming partial tie on note
    PartialTie* pt = Factory::createPartialTie(note);
    pt->setParent(note);
    pt->setEndNote(note);
    pt->setJumpPoint(jumpPoint);
    score->undoAddElement(pt);
}

void TieJumpPointList::undoRemoveTieFromScore(TieJumpPoint* jumpPoint)
{
    Note* note = jumpPoint->note();
    Score* score = note ? note->score() : nullptr;
    Tie* tie = startTie();
    if (!tie || !score) {
        return;
    }

    if (jumpPoint->followingNote()) {
        // Remove full tie and add partial tie
        if (tie->isPartialTie()) {
            return;
        }
        jumpPoint->undoSetActive(false);

        Tie::changeTieType(tie);
        return;
    }

    jumpPoint->undoSetActive(false);

    // Check if there is a full tie. If so, remove partial tie info from it
    Tie* tieBack = note->tieBack();
    if (tieBack && !tieBack->isPartialTie()) {
        tieBack->setJumpPoint(nullptr);
        return;
    }
    // Otherwise remove incoming partial tie on note
    PartialTie* pt = note->incomingPartialTie();
    if (!pt) {
        return;
    }
    score->undoRemoveElement(pt);
}
