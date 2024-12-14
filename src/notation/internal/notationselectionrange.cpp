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
#include <cfloat>

#include "notationselectionrange.h"

#include "utilities/scorerangeutilities.h"

#include "engraving/dom/segment.h"
#include "engraving/dom/measure.h"
#include "engraving/dom/chordrest.h"
#include "engraving/dom/staff.h"

using namespace mu::notation;
using namespace mu::engraving;

NotationSelectionRange::NotationSelectionRange(IGetScore* getScore)
    : m_getScore(getScore)
{
}

staff_idx_t NotationSelectionRange::startStaffIndex() const
{
    const mu::engraving::Selection& selection = score()->selection();
    return selection.staffStart();
}

Fraction NotationSelectionRange::startTick() const
{
    const mu::engraving::Selection& selection = score()->selection();
    return selection.tickStart();
}

staff_idx_t NotationSelectionRange::endStaffIndex() const
{
    const mu::engraving::Selection& selection = score()->selection();
    return selection.staffEnd();
}

Fraction NotationSelectionRange::endTick() const
{
    const mu::engraving::Selection& selection = score()->selection();
    return selection.tickEnd();
}

NotationSelectionRange::MeasureRange NotationSelectionRange::measureRange() const
{
    MeasureRange range;
    score()->selection().measureRange(&range.startMeasure, &range.endMeasure);

    return range;
}

std::vector<muse::RectF> NotationSelectionRange::boundingArea() const
{
    return ScoreRangeUtilities::boundingArea(score(),
                                             rangeStartSegment(), rangeEndSegment(),
                                             startStaffIndex(), endStaffIndex());
}

bool NotationSelectionRange::containsPoint(const PointF& point) const
{
    for (const muse::RectF& area : boundingArea()) {
        if (area.contains(point)) {
            return true;
        }
    }

    return false;
}

/// When `item` is a Measure, use `staffIdx` to query whether a specific staff is contained in the selection.
bool NotationSelectionRange::containsItem(const EngravingItem* item, engraving::staff_idx_t staffIdx) const
{
    Fraction itemTick = item->tick();
    Fraction selectionStartTick = startTick();
    Fraction selectionEndTick = endTick();

    if (itemTick < selectionStartTick || itemTick >= selectionEndTick) {
        return false;
    }

    if (item->isMeasure()) {
        if (staffIdx != muse::nidx) {
            return startStaffIndex() <= staffIdx && staffIdx < endStaffIndex();
        }

        return true;
    }

    track_idx_t itemTrack = item->track();
    track_idx_t selectionStartTrack = VOICES * startStaffIndex();
    track_idx_t selectionEndTrack = VOICES * (endStaffIndex() - 1) + VOICES;

    return itemTrack >= selectionStartTrack && itemTrack < selectionEndTrack;
}

std::vector<const Part*> NotationSelectionRange::selectedParts() const
{
    std::vector<const Part*> result;

    if (!score()->selection().isRange()) {
        return result;
    }

    staff_idx_t startStaffIndex = this->startStaffIndex();
    staff_idx_t endStaffIndex = this->endStaffIndex();

    for (staff_idx_t i = startStaffIndex; i < endStaffIndex; ++i) {
        const Staff* staff = score()->staff(i);

        if (staff && staff->part()) {
            result.push_back(staff->part());
        }
    }

    return result;
}

mu::engraving::Score* NotationSelectionRange::score() const
{
    return m_getScore->score();
}

mu::engraving::Segment* NotationSelectionRange::rangeStartSegment() const
{
    mu::engraving::Segment* startSegment = score()->selection().startSegment();

    if (!startSegment) {
        return nullptr;
    }

    if (!startSegment->enabled()) {
        startSegment = startSegment->next1MMenabled();
    }

    if (!startSegment->measure()->system()) {
        const Measure* mmr = startSegment->measure()->coveringMMRestOrThis();
        if (!mmr || mmr->system()) {
            return nullptr;
        }
        startSegment = mmr->first(mu::engraving::SegmentType::ChordRest);
    }

    return startSegment;
}

mu::engraving::Segment* NotationSelectionRange::rangeEndSegment() const
{
    mu::engraving::Segment* endSegment = score()->selection().endSegment();

    if (endSegment && !endSegment->enabled()) {
        endSegment = endSegment->next1MMenabled();
    }

    if (!endSegment) {
        endSegment = score()->lastSegmentMM();
    }

    return endSegment;
}
