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

NotationSelectionRange::NotationSelectionRange(IGetScore* getScore)
    : m_getScore(getScore)
{
}

int NotationSelectionRange::startStaffIndex() const
{
    const Ms::Selection& selection = score()->selection();
    return selection.staffStart();
}

Fraction NotationSelectionRange::startTick() const
{
    const Ms::Selection& selection = score()->selection();
    return selection.tickStart();
}

int NotationSelectionRange::endStaffIndex() const
{
    const Ms::Selection& selection = score()->selection();
    return selection.staffEnd();
}

Fraction NotationSelectionRange::endTick() const
{
    const Ms::Selection& selection = score()->selection();
    return selection.tickEnd();
}

int NotationSelectionRange::startMeasureIndex() const
{
    return measureRange().startIndex;
}

int NotationSelectionRange::endMeasureIndex() const
{
    return measureRange().endIndex;
}

std::vector<mu::RectF> NotationSelectionRange::boundingArea() const
{
    const Ms::Selection& selection = score()->selection();
    if (!selection.isRange()) {
        return {};
    }

    const Ms::Segment* startSegment = rangeStartSegment();
    const Ms::Segment* endSegment = rangeEndSegment();
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
        const Ms::System* sectionSystem = rangeSection.system;
        const Ms::Segment* sectionStartSegment = rangeSection.startSegment;
        const Ms::Segment* sectionEndSegment = rangeSection.endSegment;

        const Ms::SysStaff* segmentFirstStaff = sectionSystem->staff(score()->selection().staffStart());
        const Ms::SysStaff* segmentLastStaff = sectionSystem->staff(lastStaff);

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

Ms::Score* NotationSelectionRange::score() const
{
    return m_getScore->score();
}

Ms::Segment* NotationSelectionRange::rangeStartSegment() const
{
    Ms::Segment* startSegment = score()->selection().startSegment();

    startSegment->measure()->firstEnabled();

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
        startSegment = mmr->first(Ms::SegmentType::ChordRest);
    }

    return startSegment;
}

Ms::Segment* NotationSelectionRange::rangeEndSegment() const
{
    Ms::Segment* endSegment = score()->selection().endSegment();

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
    for (int i = score()->selection().staffEnd() - 1; i >= 0; --i) {
        if (score()->staff(i)->show()) {
            return i;
        }
    }

    return 0;
}

std::vector<NotationSelectionRange::RangeSection> NotationSelectionRange::splitRangeBySections(const Ms::Segment* rangeStartSegment,
                                                                                               const Ms::Segment* rangeEndSegment) const
{
    std::vector<RangeSection> sections;

    const Ms::Segment* startSegment = rangeStartSegment;
    for (const Ms::Segment* segment = rangeStartSegment; segment && (segment != rangeEndSegment);) {
        Ms::System* currentSegmentSystem = segment->measure()->system();

        Ms::Segment* nextSegment = segment->next1MMenabled();
        Ms::System* nextSegmentSystem = nextSegment->measure()->system();

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
    const Ms::System* segmentSystem = selection.system;
    const Ms::Segment* startSegment = selection.startSegment;
    const Ms::Segment* endSegment = selection.endSegment;

    Ms::SysStaff* segmentFirstStaff = segmentSystem->staff(score()->selection().staffStart());

    Ms::SkylineLine north = segmentFirstStaff->skyline().north();
    int maxY = INT_MAX;
    for (Ms::SkylineSegment segment: north) {
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
    const Ms::System* segmentSystem = selection.system;
    const Ms::Segment* startSegment = selection.startSegment;
    const Ms::Segment* endSegment = selection.endSegment;

    int lastStaff = selectionLastVisibleStaff();
    Ms::SysStaff* segmentLastStaff = segmentSystem->staff(lastStaff);

    Ms::SkylineLine south = segmentLastStaff->skyline().south();
    int minY = INT_MIN;
    for (Ms::SkylineSegment segment: south) {
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

NotationSelectionRange::MeasureRange NotationSelectionRange::measureRange() const
{
    const Ms::Selection& selection = score()->selection();
    Measure* startMeasure = nullptr;
    Measure* endMeasure = nullptr;
    selection.measureRange(&startMeasure, &endMeasure);

    MeasureRange range;
    range.startIndex = startMeasure ? startMeasure->index() : 0;
    range.endIndex = endMeasure ? endMeasure->index() : 0;

    return range;
}
