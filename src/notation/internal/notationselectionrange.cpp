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
#include "notationselectionrange.h"

#include "libmscore/masterscore.h"
#include "libmscore/segment.h"
#include "libmscore/measure.h"
#include "libmscore/system.h"
#include "libmscore/chordrest.h"
#include "libmscore/skyline.h"

#include "log.h"

static constexpr int SELECTION_SIDE_PADDING = 8;

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

std::vector<mu::RectF> NotationSelectionRange::boundingArea() const
{
    const mu::engraving::Selection& selection = score()->selection();
    if (!selection.isRange()) {
        return {};
    }

    const mu::engraving::Segment* startSegment = rangeStartSegment();
    const mu::engraving::Segment* endSegment = rangeEndSegment();
    if (!endSegment) {
        endSegment = score()->lastSegment();
    }

    if (!startSegment || !endSegment || startSegment->tick() > endSegment->tick()) {
        return {};
    }

    std::vector<RectF> result;

    std::vector<RangeSection> rangeSections = splitRangeBySections(startSegment, endSegment);

    int lastStaff = selectionLastVisibleStaff();

    for (const RangeSection& rangeSection: rangeSections) {
        const mu::engraving::System* sectionSystem = rangeSection.system;
        const mu::engraving::Segment* sectionStartSegment = rangeSection.startSegment;
        const mu::engraving::Segment* sectionEndSegment = rangeSection.endSegment;

        const mu::engraving::SysStaff* segmentFirstStaff = sectionSystem->staff(score()->selection().staffStart());
        const mu::engraving::SysStaff* segmentLastStaff = sectionSystem->staff(lastStaff);

        int topY = sectionElementsMaxY(rangeSection);
        int bottomY = sectionElementsMinY(rangeSection);

        double x1 = sectionStartSegment->pagePos().x() - SELECTION_SIDE_PADDING;
        double x2 = sectionEndSegment->pageBoundingRect().topRight().x();
        double y1 = topY + segmentFirstStaff->y() + sectionStartSegment->pagePos().y() - SELECTION_SIDE_PADDING;
        double y2 = bottomY + segmentLastStaff->y() + sectionStartSegment->pagePos().y() + SELECTION_SIDE_PADDING;

        if (sectionStartSegment->measure()->first() == sectionStartSegment) {
            x1 = sectionStartSegment->measure()->pagePos().x();
        }

        RectF rect = RectF(PointF(x1, y1), PointF(x2, y2)).translated(sectionSystem->page()->pos());
        result.push_back(rect);
    }

    return result;
}

bool NotationSelectionRange::containsPoint(const PointF& point) const
{
    for (const mu::RectF& area : boundingArea()) {
        if (area.contains(point)) {
            return true;
        }
    }

    return false;
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
        const Measure* mmr = startSegment->measure()->mmRest1();
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

    if (!endSegment) {
        return nullptr;
    }

    if (!endSegment->enabled()) {
        endSegment = endSegment->next1MMenabled();
    }

    return endSegment;
}

int NotationSelectionRange::selectionLastVisibleStaff() const
{
    for (int i = static_cast<int>(score()->selection().staffEnd()) - 1; i >= 0; --i) {
        if (score()->staff(i)->show()) {
            return i;
        }
    }

    return 0;
}

std::vector<NotationSelectionRange::RangeSection> NotationSelectionRange::splitRangeBySections(
    const mu::engraving::Segment* rangeStartSegment,
    const mu::engraving::Segment* rangeEndSegment)
const
{
    std::vector<RangeSection> sections;

    const mu::engraving::Segment* startSegment = rangeStartSegment;
    Fraction rangeEndTick = rangeEndSegment->tick();
    for (const mu::engraving::Segment* segment = rangeStartSegment;
         segment && segment != rangeEndSegment && segment->tick() <= rangeEndTick;) {
        mu::engraving::System* currentSegmentSystem = segment->measure()->system();

        mu::engraving::Segment* nextSegment = segment->next1MMenabled();
        mu::engraving::System* nextSegmentSystem = nextSegment->measure()->system();

        if (!nextSegmentSystem) {
            const Measure* mmr = nextSegment->measure()->mmRest1();
            if (mmr) {
                nextSegmentSystem = mmr->system();
            }
            if (!nextSegmentSystem) {
                break;
            }
        }

        if (nextSegmentSystem != currentSegmentSystem || nextSegment == rangeEndSegment) {
            RangeSection section;
            section.system = currentSegmentSystem;
            section.startSegment = startSegment;
            section.endSegment = segment;

            sections.push_back(section);
            startSegment = nextSegment;
        }

        segment = nextSegment;
    }

    return sections;
}

int NotationSelectionRange::sectionElementsMaxY(const NotationSelectionRange::RangeSection& selection) const
{
    const mu::engraving::System* segmentSystem = selection.system;
    const mu::engraving::Segment* startSegment = selection.startSegment;
    const mu::engraving::Segment* endSegment = selection.endSegment;

    mu::engraving::SysStaff* segmentFirstStaff = segmentSystem->staff(score()->selection().staffStart());

    mu::engraving::SkylineLine north = segmentFirstStaff->skyline().north();
    int maxY = INT_MAX;
    for (mu::engraving::SkylineSegment segment: north) {
        bool ok = segment.x >= startSegment->pagePos().x() && segment.x <= endSegment->pagePos().x();
        if (!ok) {
            continue;
        }

        if (segment.y < maxY) {
            maxY = segment.y;
        }
    }

    if (maxY == INT_MAX) {
        maxY = 0;
    }

    return maxY;
}

int NotationSelectionRange::sectionElementsMinY(const NotationSelectionRange::RangeSection& selection) const
{
    const mu::engraving::System* segmentSystem = selection.system;
    const mu::engraving::Segment* startSegment = selection.startSegment;
    const mu::engraving::Segment* endSegment = selection.endSegment;

    int lastStaff = selectionLastVisibleStaff();
    mu::engraving::SysStaff* segmentLastStaff = segmentSystem->staff(lastStaff);

    mu::engraving::SkylineLine south = segmentLastStaff->skyline().south();
    int minY = INT_MIN;
    for (mu::engraving::SkylineSegment segment: south) {
        bool ok = segment.x >= startSegment->pagePos().x() && segment.x <= endSegment->pagePos().x();
        if (!ok) {
            continue;
        }

        if (segment.y > minY) {
            minY = segment.y;
        }
    }

    if (minY == INT_MIN) {
        minY = segmentLastStaff->bbox().height();
    }

    return minY;
}
